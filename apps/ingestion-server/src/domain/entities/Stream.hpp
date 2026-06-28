#pragma once
#include <chrono>
#include <optional>
#include <string>

namespace homecctv::domain {

enum class StreamStatus { Idle = 1, Connecting = 2, Streaming = 3, Error = 4 };

struct StreamInfo {
    std::string camera_id;
    StreamStatus status = StreamStatus::Idle;
    std::string codec_name;
    int width = 0;
    int height = 0;
    double fps = 0.0;
    std::chrono::system_clock::time_point started_at;
    std::string hls_playlist_url;
    std::optional<std::string> last_error;
};

}  // namespace homecctv::domain
