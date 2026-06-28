#pragma once
#include <chrono>
#include <optional>
#include <string>

namespace homecctv::domain {

enum class CameraStatus { Inactive = 1, Active = 2, Error = 3 };

enum class RtspTransport { Tcp, Udp };

struct CameraSettings {
    RtspTransport transport = RtspTransport::Tcp;
    int segment_duration_sec = 2;
    int hls_list_size = 10;
    bool recording_enabled = false;
    std::string recording_path;
};

struct Camera {
    std::string id;
    std::string name;
    std::string rtsp_url;
    CameraStatus status = CameraStatus::Inactive;
    CameraSettings settings;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    std::optional<std::string> last_error;
};

}  // namespace homecctv::domain
