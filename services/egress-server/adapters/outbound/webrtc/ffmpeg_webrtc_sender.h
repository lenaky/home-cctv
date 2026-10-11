#pragma once
#include <string>
#include <string_view>

#include "domain/ports/webrtc_egress_pipeline.h"

class FfmpegWebRtcSender : public IWebRtcEgressPipeline {
public:
    FfmpegWebRtcSender() = default;
    Result<std::string> CreateSession(CameraId camera_id, std::string_view sdp_offer) override;
    Result<void> CloseSession(std::string_view session_id) override;
};
