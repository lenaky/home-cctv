#include "HlsSegmentWriter.hpp"

#include <spdlog/spdlog.h>

namespace homecctv::adapters {

HlsSegmentWriter::HlsSegmentWriter(std::filesystem::path hls_root, std::string camera_id,
                                   std::string base_url, HlsOptions opts)
    : hls_root_(std::move(hls_root)),
      camera_id_(std::move(camera_id)),
      base_url_(std::move(base_url)),
      opts_(std::move(opts)) {}

HlsSegmentWriter::~HlsSegmentWriter() { close(); }

Result<void> HlsSegmentWriter::open(const AVStream* source_stream) {
    auto cam_dir = hls_root_ / camera_id_;
    std::filesystem::create_directories(cam_dir);

    auto playlist = (cam_dir / "index.m3u8").string();
    auto seg_pattern = (cam_dir / "seg%05d.ts").string();

    int ret = avformat_alloc_output_context2(&hls_ctx_, nullptr, "hls", playlist.c_str());
    if (ret < 0 || !hls_ctx_)
        return Result<void>::Err("Failed to create HLS output context");

    out_stream_ = avformat_new_stream(hls_ctx_, nullptr);
    if (!out_stream_) return Result<void>::Err("Failed to create HLS output stream");

    avcodec_parameters_copy(out_stream_->codecpar, source_stream->codecpar);
    out_stream_->time_base = source_stream->time_base;
    // HLS/TS requires hvc1 tag for HEVC; without it, some players refuse to decode
    if (out_stream_->codecpar->codec_id == AV_CODEC_ID_HEVC)
        out_stream_->codecpar->codec_tag = MKTAG('h', 'v', 'c', '1');

    AVDictionary* hls_opts = nullptr;
    av_dict_set_int(&hls_opts, "hls_time", opts_.segment_duration, 0);
    av_dict_set_int(&hls_opts, "hls_list_size", opts_.list_size, 0);
    av_dict_set(&hls_opts, "hls_segment_filename", seg_pattern.c_str(), 0);
    av_dict_set(&hls_opts, "hls_flags", "delete_segments+append_list+independent_segments", 0);

    if (opts_.low_latency) {
        av_dict_set_int(&hls_opts, "hls_time", 1, 0);
        av_dict_set_int(&hls_opts, "hls_list_size", 3, 0);
    }

    ret = avformat_write_header(hls_ctx_, &hls_opts);
    av_dict_free(&hls_opts);

    if (ret < 0) return Result<void>::Err("Failed to write HLS header");

    errored_ = false;
    next_dts_ = 0;
    last_dts_ = AV_NOPTS_VALUE;
    spdlog::info("HLS writer opened: {}", playlist);
    return Result<void>::Ok();
}

Result<void> HlsSegmentWriter::writePacket(const AVPacket* pkt,
                                            const AVRational& src_time_base) {
    if (!hls_ctx_) return Result<void>::Err("HLS context not initialized");
    if (errored_) return Result<void>::Err("HLS context in error state");

    AVPacket* out_pkt = av_packet_clone(pkt);
    if (!out_pkt) return Result<void>::Err("Failed to clone packet");

    out_pkt->stream_index = 0;

    // Fix partial missing timestamps before rescaling
    if (out_pkt->pts == AV_NOPTS_VALUE && out_pkt->dts != AV_NOPTS_VALUE)
        out_pkt->pts = out_pkt->dts;
    else if (out_pkt->dts == AV_NOPTS_VALUE && out_pkt->pts != AV_NOPTS_VALUE)
        out_pkt->dts = out_pkt->pts;

    av_packet_rescale_ts(out_pkt, src_time_base, out_stream_->time_base);

    // If both timestamps are still missing, synthesize from running counter
    if (out_pkt->pts == AV_NOPTS_VALUE)
        out_pkt->pts = out_pkt->dts = next_dts_;

    // Enforce strictly monotonically increasing DTS (muxer requirement)
    // last_dts_ starts at AV_NOPTS_VALUE (INT64_MIN), so first packet always passes
    if (out_pkt->dts <= last_dts_) {
        int64_t pts_dts_diff = (out_pkt->pts != AV_NOPTS_VALUE) ? (out_pkt->pts - out_pkt->dts) : 0;
        out_pkt->dts = last_dts_ + 1;
        out_pkt->pts = out_pkt->dts + pts_dts_diff;
    }
    last_dts_ = out_pkt->dts;

    // Advance synthetic counter
    int64_t dur = (out_pkt->duration > 0)
                      ? out_pkt->duration
                      : av_rescale_q(1, AVRational{1, 30}, out_stream_->time_base);
    next_dts_ = out_pkt->dts + dur;

    int ret = av_interleaved_write_frame(hls_ctx_, out_pkt);
    av_packet_free(&out_pkt);

    if (ret < 0) {
        errored_ = true;
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        return Result<void>::Err(std::string("HLS write failed: ") + errbuf);
    }
    return Result<void>::Ok();
}

Result<void> HlsSegmentWriter::close() {
    if (hls_ctx_) {
        av_write_trailer(hls_ctx_);
        avformat_free_context(hls_ctx_);
        hls_ctx_ = nullptr;
        out_stream_ = nullptr;
    }
    errored_ = false;
    last_dts_ = AV_NOPTS_VALUE;
    return Result<void>::Ok();
}

std::string HlsSegmentWriter::getPlaylistUrl() const {
    return base_url_ + "/hls/" + camera_id_ + "/index.m3u8";
}

}  // namespace homecctv::adapters
