#include "adapters/outbound/webrtc/ffmpeg_webrtc_sender.h"

// Real implementation will connect to ingestion-server's per-camera local IPC
// packet feed (Unix Domain Socket on macOS / Named Pipe on Windows) for
// genuine low-latency media, not HLS segment replay — not implemented here.

Result<std::string> FfmpegWebRtcSender::CreateSession(CameraId camera_id,
                                                      std::string_view sdp_offer) {
    (void)camera_id;
    (void)sdp_offer;
    return Result<std::string>::Err(
        Error{ErrorCode::NotImplemented, "FfmpegWebRtcSender::CreateSession not implemented"});
}

Result<void> FfmpegWebRtcSender::CloseSession(std::string_view session_id) {
    (void)session_id;
    return Result<void>::Err(
        Error{ErrorCode::NotImplemented, "FfmpegWebRtcSender::CloseSession not implemented"});
}
