#include "RtspReceiver.hpp"

#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

namespace homecctv::adapters {

RtspReceiver::RtspReceiver(std::string url, RtspOptions opts)
    : url_(std::move(url)), opts_(std::move(opts)) {}

RtspReceiver::~RtspReceiver() { stop(); }

void RtspReceiver::start(PacketCallback on_packet, ErrorCallback on_error) {
    running_ = true;
    receive_thread_ = std::thread([this,
                                    on_packet = std::move(on_packet),
                                    on_error = std::move(on_error)]() mutable {
        int attempts = 0;
        while (running_) {
            receiveLoop(on_packet, on_error);
            if (!running_) break;
            ++attempts;
            if (attempts >= opts_.max_reconnect_attempts) {
                spdlog::error("RTSP max reconnect attempts reached: {}", url_);
                on_error("Max reconnect attempts reached");
                break;
            }
            spdlog::info("RTSP reconnecting in {}ms (attempt {}/{})",
                         opts_.reconnect_delay_ms, attempts, opts_.max_reconnect_attempts);
            on_error("Reconnecting...");
            std::this_thread::sleep_for(std::chrono::milliseconds(opts_.reconnect_delay_ms));
        }
        running_ = false;
    });
}

void RtspReceiver::stop() {
    running_ = false;
    if (receive_thread_.joinable()) receive_thread_.join();
    closeInput();
}

bool RtspReceiver::openInput() {
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "rtsp_transport", opts_.transport.c_str(), 0);
    av_dict_set_int(&opts, "timeout", opts_.timeout_us, 0);
    av_dict_set_int(&opts, "stimeout", opts_.timeout_us, 0);
    av_dict_set_int(&opts, "analyzeduration", 1'000'000, 0);
    av_dict_set_int(&opts, "probesize", 1'000'000, 0);

    int ret = avformat_open_input(&fmt_ctx_, url_.c_str(), nullptr, &opts);
    av_dict_free(&opts);

    if (ret < 0) {
        char err[AV_ERROR_MAX_STRING_SIZE]{};
        av_strerror(ret, err, sizeof(err));
        spdlog::error("RTSP open failed [{}]: {}", url_, err);
        return false;
    }

    if (avformat_find_stream_info(fmt_ctx_, nullptr) < 0) {
        spdlog::error("RTSP stream info not found: {}", url_);
        closeInput();
        return false;
    }

    video_stream_idx_ =
        av_find_best_stream(fmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_stream_idx_ < 0) {
        spdlog::error("RTSP no video stream: {}", url_);
        closeInput();
        return false;
    }

    auto* st = fmt_ctx_->streams[video_stream_idx_];
    spdlog::info("RTSP connected: {} — codec={} {}x{}",
                 url_,
                 avcodec_get_name(st->codecpar->codec_id),
                 st->codecpar->width,
                 st->codecpar->height);
    return true;
}

void RtspReceiver::closeInput() {
    if (fmt_ctx_) {
        avformat_close_input(&fmt_ctx_);
        fmt_ctx_ = nullptr;
    }
}

void RtspReceiver::receiveLoop(PacketCallback on_packet, ErrorCallback on_error) {
    if (!openInput()) {
        on_error("Failed to open RTSP stream");
        return;
    }

    AVPacket* pkt = av_packet_alloc();

    while (running_) {
        int ret = av_read_frame(fmt_ctx_, pkt);

        if (ret == AVERROR(EAGAIN)) {
            av_packet_unref(pkt);
            continue;
        }

        if (ret < 0) {
            if (!running_) break;
            char err[AV_ERROR_MAX_STRING_SIZE]{};
            av_strerror(ret, err, sizeof(err));
            spdlog::warn("RTSP read error [{}]: {}", url_, err);
            on_error(std::string("Stream read error: ") + err);
            break;
        }

        if (pkt->stream_index == video_stream_idx_) {
            on_packet(pkt, fmt_ctx_->streams[video_stream_idx_]);
        }

        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
    closeInput();
}

}  // namespace homecctv::adapters
