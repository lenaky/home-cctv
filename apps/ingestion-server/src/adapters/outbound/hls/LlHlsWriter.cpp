#include "LlHlsWriter.hpp"

#include <format>
#include <fstream>
#include <sstream>

#include <spdlog/spdlog.h>

namespace homecctv::adapters {

LlHlsWriter::LlHlsWriter(std::filesystem::path hls_root, std::string camera_id,
                         std::string base_url, LlHlsConfig cfg)
    : hls_root_(std::move(hls_root)),
      camera_id_(std::move(camera_id)),
      base_url_(std::move(base_url)),
      cfg_(std::move(cfg)) {}

LlHlsWriter::~LlHlsWriter() { close(); }

// ---------------------------------------------------------------------------
// ISegmentWriter
// ---------------------------------------------------------------------------

Result<void> LlHlsWriter::open(const AVStream* source_stream) {
    cam_dir_ = hls_root_ / camera_id_;
    std::filesystem::create_directories(cam_dir_);

    src_tb_ = source_stream->time_base;

    writeInitSegment(source_stream);

    current_seg_seq_ = 1;
    openSegment(1);
    writePlaylist();

    initialized_ = true;
    errored_     = false;
    spdlog::info("LL-HLS writer opened for camera {}", camera_id_);
    return Result<void>::Ok();
}

Result<void> LlHlsWriter::writePacket(const AVPacket* pkt, const AVRational& src_time_base) {
    if (!initialized_) return Result<void>::Err("LlHlsWriter not opened");
    if (errored_)      return Result<void>::Err("LlHlsWriter in error state");

    AVPacket* out_pkt = av_packet_clone(pkt);
    if (!out_pkt) return Result<void>::Err("Failed to clone packet");

    out_pkt->stream_index = 0;

    // Fix partial missing timestamps before rescaling
    if (out_pkt->pts == AV_NOPTS_VALUE && out_pkt->dts != AV_NOPTS_VALUE)
        out_pkt->pts = out_pkt->dts;
    else if (out_pkt->dts == AV_NOPTS_VALUE && out_pkt->pts != AV_NOPTS_VALUE)
        out_pkt->dts = out_pkt->pts;

    av_packet_rescale_ts(out_pkt, src_time_base, out_stream_->time_base);

    if (out_pkt->pts == AV_NOPTS_VALUE)
        out_pkt->pts = out_pkt->dts = next_dts_;

    // Enforce strictly monotonically increasing DTS
    if (out_pkt->dts <= last_dts_) {
        int64_t diff = (out_pkt->pts != AV_NOPTS_VALUE) ? (out_pkt->pts - out_pkt->dts) : 0;
        out_pkt->dts = last_dts_ + 1;
        out_pkt->pts = out_pkt->dts + diff;
    }
    last_dts_ = out_pkt->dts;

    // Ensure duration is always positive (some cameras send 0 or negative durations)
    int64_t fallback_dur = av_rescale_q(1, AVRational{1, 30}, out_stream_->time_base);
    if (out_pkt->duration <= 0) out_pkt->duration = fallback_dur;
    next_dts_ = out_pkt->dts + out_pkt->duration;

    double pkt_sec = av_q2d(out_stream_->time_base) * static_cast<double>(out_pkt->duration);

    bool is_key = (pkt->flags & AV_PKT_FLAG_KEY) != 0;

    // Split segment on keyframe boundary; hard limit at 2× segment_duration to avoid
    // infinitely long segments on cameras with long GOPs.
    bool seg_time_reached = (current_seg_duration_ >= cfg_.segment_duration && is_key) ||
                             (current_seg_duration_ >= cfg_.segment_duration * 2.0);
    if (seg_time_reached) {
        if (current_part_duration_ > 0)
            flushPart(next_part_independent_);
        finalizeSegment();
        openSegment(++current_seg_seq_);
        next_part_independent_ = is_key;
        current_part_duration_ = 0.0;
    } else if (is_key && current_part_duration_ > 0) {
        // Flush buffered frames BEFORE writing the keyframe so the keyframe becomes
        // the first sample of the next part (enabling INDEPENDENT=YES on that part).
        flushPart(next_part_independent_);
        next_part_independent_ = true;
        current_part_duration_ = 0.0;
    }

    // Write packet to current segment (frag_custom: av_write_frame without flush)
    int ret = av_write_frame(ctx_, out_pkt);
    av_packet_free(&out_pkt);

    if (ret < 0) {
        errored_ = true;
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        return Result<void>::Err(std::string("LlHls write failed: ") + errbuf);
    }

    current_part_duration_   += pkt_sec;
    current_seg_duration_    += pkt_sec;

    // Flush a part when duration threshold is reached
    if (current_part_duration_ >= cfg_.part_duration) {
        flushPart(next_part_independent_);
        next_part_independent_ = false;
        current_part_duration_ = 0.0;
    }

    return Result<void>::Ok();
}

Result<void> LlHlsWriter::close() {
    if (!initialized_) return Result<void>::Ok();

    if (!errored_) {
        if (current_part_duration_ > 0)
            flushPart(next_part_independent_);
        finalizeSegment();
        writePlaylist(/*end_of_stream=*/true);
    }

    if (ctx_) {
        if (ctx_->pb) avio_closep(&ctx_->pb);
        avformat_free_context(ctx_);
        ctx_        = nullptr;
        out_stream_ = nullptr;
    }

    initialized_          = false;
    errored_              = false;
    last_dts_             = AV_NOPTS_VALUE;
    next_dts_             = 0;
    current_seg_duration_ = 0.0;
    current_part_duration_= 0.0;
    completed_segs_.clear();
    return Result<void>::Ok();
}

std::string LlHlsWriter::getPlaylistUrl() const {
    return base_url_ + "/hls/" + camera_id_ + "/index.m3u8";
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void LlHlsWriter::writeInitSegment(const AVStream* source_stream) {
    // Allocate mp4 muxer context (no output file yet)
    avformat_alloc_output_context2(&ctx_, nullptr, "mp4", nullptr);

    out_stream_ = avformat_new_stream(ctx_, nullptr);
    avcodec_parameters_copy(out_stream_->codecpar, source_stream->codecpar);
    out_stream_->time_base = source_stream->time_base;
    if (out_stream_->codecpar->codec_id == AV_CODEC_ID_HEVC)
        out_stream_->codecpar->codec_tag = MKTAG('h', 'v', 'c', '1');

    // Write ftyp+moov to init.mp4 only
    auto init_path = (cam_dir_ / "init.mp4").string();
    avio_open(&ctx_->pb, init_path.c_str(), AVIO_FLAG_WRITE);

    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "movflags", "frag_custom+empty_moov+default_base_moof", 0);
    avformat_write_header(ctx_, &opts);
    av_dict_free(&opts);

    avio_closep(&ctx_->pb);  // Close init.mp4; ctx_ retains muxer state
    spdlog::debug("LL-HLS init.mp4 written: {}", init_path);
}

void LlHlsWriter::openSegment(int seq) {
    current_seg_.seq      = seq;
    current_seg_.filename = std::format("seg{:05d}.mp4", seq);
    current_seg_.parts.clear();
    current_seg_.duration = 0.0;

    auto seg_path = (cam_dir_ / current_seg_.filename).string();
    avio_open(&ctx_->pb, seg_path.c_str(), AVIO_FLAG_WRITE);
    part_start_offset_ = 0;
}

void LlHlsWriter::flushPart(bool independent) {
    int64_t start = part_start_offset_;

    // Flush buffered frames as one fMP4 fragment (moof+mdat)
    [[maybe_unused]] int flush_ret = av_write_frame(ctx_, nullptr);
    avio_flush(ctx_->pb);

    int64_t end    = avio_tell(ctx_->pb);
    int64_t length = end - start;
    if (length <= 0) return;

    current_seg_.parts.push_back({current_part_duration_, start, length, independent});
    part_start_offset_ = end;

    writePlaylist();
}

void LlHlsWriter::finalizeSegment() {
    if (ctx_->pb) {
        avio_flush(ctx_->pb);
        avio_closep(&ctx_->pb);
    }

    current_seg_.duration = current_seg_duration_;
    completed_segs_.push_back(current_seg_);

    // Delete oldest segment files beyond list_size
    while (static_cast<int>(completed_segs_.size()) > cfg_.list_size) {
        std::filesystem::remove(cam_dir_ / completed_segs_.front().filename);
        completed_segs_.erase(completed_segs_.begin());
    }

    current_seg_duration_ = 0.0;
}

void LlHlsWriter::writePlaylist(bool end_of_stream) {
    // Determine the first sequence number in the playlist
    int first_seq = completed_segs_.empty() ? current_seg_.seq
                                            : completed_segs_.front().seq;

    std::string m3u8;
    m3u8.reserve(4096);
    m3u8 += "#EXTM3U\n";
    m3u8 += "#EXT-X-VERSION:9\n";
    // TARGETDURATION must be >= every EXTINF in the playlist; compute from actuals.
    int target_dur = static_cast<int>(std::ceil(cfg_.segment_duration));
    for (const auto& seg : completed_segs_)
        target_dur = std::max(target_dur, static_cast<int>(std::ceil(seg.duration)));
    m3u8 += std::format("#EXT-X-TARGETDURATION:{}\n", target_dur);
    m3u8 += std::format("#EXT-X-PART-INF:PART-TARGET={:.3f}\n", cfg_.part_duration);
    m3u8 += std::format(
        "#EXT-X-SERVER-CONTROL:CAN-BLOCK-RELOAD=YES,PART-HOLD-BACK={:.3f}\n",
        cfg_.part_duration * 3.0);
    m3u8 += std::format("#EXT-X-MEDIA-SEQUENCE:{}\n", first_seq);
    m3u8 += "#EXT-X-MAP:URI=\"init.mp4\"\n\n";

    // Complete segments (parts first, then EXTINF)
    for (const auto& seg : completed_segs_) {
        for (const auto& p : seg.parts) {
            m3u8 += std::format("#EXT-X-PART:DURATION={:.3f}", p.duration);
            if (p.independent) m3u8 += ",INDEPENDENT=YES";
            m3u8 += std::format(",URI=\"{}\",BYTERANGE={}@{}\n",
                                seg.filename, p.byte_length, p.byte_offset);
        }
        m3u8 += std::format("#EXTINF:{:.3f},\n{}\n", seg.duration, seg.filename);
    }

    // In-progress segment parts
    for (const auto& p : current_seg_.parts) {
        m3u8 += std::format("#EXT-X-PART:DURATION={:.3f}", p.duration);
        if (p.independent) m3u8 += ",INDEPENDENT=YES";
        m3u8 += std::format(",URI=\"{}\",BYTERANGE={}@{}\n",
                            current_seg_.filename, p.byte_length, p.byte_offset);
    }

    // Preload hint for the next expected part (skipped on end-of-stream)
    if (!end_of_stream) {
        int64_t next_start = current_seg_.parts.empty() ? 0
            : current_seg_.parts.back().byte_offset + current_seg_.parts.back().byte_length;
        m3u8 += std::format(
            "#EXT-X-PRELOAD-HINT:TYPE=PART,URI=\"{}\",BYTERANGE-START={}\n",
            current_seg_.filename, next_start);
    }

    if (end_of_stream) m3u8 += "#EXT-X-ENDLIST\n";

    // Atomic write: write to .tmp then rename
    auto tmp_path      = cam_dir_ / "index.m3u8.tmp";
    auto playlist_path = cam_dir_ / "index.m3u8";

    std::ofstream f(tmp_path);
    f << m3u8;
    f.close();
    std::filesystem::rename(tmp_path, playlist_path);
}

}  // namespace homecctv::adapters
