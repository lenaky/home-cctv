#include "FMp4Writer.hpp"

#include <chrono>
#include <format>

#include <spdlog/spdlog.h>

namespace homecctv::adapters {

// H.264 and H.265 remux cleanly to fMP4; others may not
bool FMp4Writer::isCompatibleCodec(AVCodecID codec_id) {
    return codec_id == AV_CODEC_ID_H264 || codec_id == AV_CODEC_ID_HEVC;
}

FMp4Writer::FMp4Writer(std::filesystem::path recording_dir, std::string camera_id,
                       FMp4Options opts)
    : recording_dir_(std::move(recording_dir)),
      camera_id_(std::move(camera_id)),
      opts_(std::move(opts)) {}

FMp4Writer::~FMp4Writer() { close(); }

Result<void> FMp4Writer::open(const AVStream* source_stream) {
    src_stream_ = source_stream;
    use_fmp4_ = isCompatibleCodec(source_stream->codecpar->codec_id);

    auto cam_dir = recording_dir_ / camera_id_;
    std::filesystem::create_directories(cam_dir);

    return openSegment();
}

Result<void> FMp4Writer::openSegment() {
    if (ctx_) {
        av_write_trailer(ctx_);
        avformat_free_context(ctx_);
        ctx_ = nullptr;
    }

    auto now = std::chrono::system_clock::now();
    auto ts = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    std::string filename;
    std::string format;
    if (use_fmp4_) {
        filename = std::format("{}_{:04d}_{}.mp4", camera_id_, segment_index_, ts);
        format = "mp4";
    } else {
        filename = std::format("{}_{:04d}_{}.ts", camera_id_, segment_index_, ts);
        format = "mpegts";
    }

    auto path = (recording_dir_ / camera_id_ / filename).string();

    int ret = avformat_alloc_output_context2(&ctx_, nullptr, format.c_str(), path.c_str());
    if (ret < 0 || !ctx_) return Result<void>::Err("Failed to create recording context");

    out_stream_ = avformat_new_stream(ctx_, nullptr);
    if (!out_stream_) return Result<void>::Err("Failed to create recording stream");

    avcodec_parameters_copy(out_stream_->codecpar, src_stream_->codecpar);
    out_stream_->time_base = src_stream_->time_base;

    AVDictionary* mux_opts = nullptr;
    if (use_fmp4_) {
        av_dict_set(&mux_opts, "movflags",
                    "frag_keyframe+empty_moov+default_base_moof+faststart", 0);
    }

    ret = avio_open(&ctx_->pb, path.c_str(), AVIO_FLAG_WRITE);
    if (ret < 0) { av_dict_free(&mux_opts); return Result<void>::Err("Failed to open file: " + path); }

    ret = avformat_write_header(ctx_, &mux_opts);
    av_dict_free(&mux_opts);
    if (ret < 0) return Result<void>::Err("Failed to write recording header");

    segment_start_pts_ = AV_NOPTS_VALUE;
    ++segment_index_;
    spdlog::info("Recording segment opened: {}", path);
    return Result<void>::Ok();
}

Result<void> FMp4Writer::writePacket(const AVPacket* pkt, const AVRational& src_time_base) {
    if (!ctx_) return Result<void>::Err("Recording context not open");

    if (segment_start_pts_ == AV_NOPTS_VALUE) segment_start_pts_ = pkt->pts;

    double elapsed_sec =
        (pkt->pts - segment_start_pts_) *
        av_q2d(src_time_base);

    if (elapsed_sec >= opts_.segment_duration_sec) {
        if (auto r = rotate(); r.is_err()) return r;
    }

    AVPacket* out_pkt = av_packet_clone(pkt);
    out_pkt->stream_index = 0;
    av_packet_rescale_ts(out_pkt, src_time_base, out_stream_->time_base);

    int ret = av_interleaved_write_frame(ctx_, out_pkt);
    av_packet_free(&out_pkt);

    if (ret < 0) return Result<void>::Err("Failed to write recording packet");
    return Result<void>::Ok();
}

Result<void> FMp4Writer::rotate() { return openSegment(); }

Result<void> FMp4Writer::close() {
    if (ctx_) {
        av_write_trailer(ctx_);
        if (ctx_->pb) avio_closep(&ctx_->pb);
        avformat_free_context(ctx_);
        ctx_ = nullptr;
        out_stream_ = nullptr;
    }
    return Result<void>::Ok();
}

}  // namespace homecctv::adapters
