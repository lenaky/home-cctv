#pragma once
#include <atomic>
#include <functional>
#include <string>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace homecctv::adapters {

struct RtspOptions {
    std::string transport = "tcp";
    int timeout_us = 5'000'000;
    int reconnect_delay_ms = 3000;
    int max_reconnect_attempts = 5;
};

using PacketCallback = std::function<void(AVPacket*, const AVStream*)>;
using ErrorCallback = std::function<void(const std::string&)>;

class RtspReceiver {
public:
    RtspReceiver(std::string url, RtspOptions opts = {});
    ~RtspReceiver();

    void start(PacketCallback on_packet, ErrorCallback on_error);
    void stop();
    bool isRunning() const noexcept { return running_.load(); }

private:
    void receiveLoop(PacketCallback on_packet, ErrorCallback on_error);
    bool openInput();
    void closeInput();

    std::string url_;
    RtspOptions opts_;
    AVFormatContext* fmt_ctx_ = nullptr;
    int video_stream_idx_ = -1;
    std::atomic<bool> running_{false};
    std::thread receive_thread_;
};

}  // namespace homecctv::adapters
