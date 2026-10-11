#pragma once
#include <string>
#include <string_view>

#include "domain/ports/webrtc_egress_pipeline.h"
#include "domain/result.h"
#include "domain/types.h"

class CreateWebRtcSessionUseCase {
public:
    explicit CreateWebRtcSessionUseCase(IWebRtcEgressPipeline& pipeline);
    Result<std::string> Execute(CameraId camera_id, std::string_view sdp_offer);

private:
    IWebRtcEgressPipeline& pipeline_;
};
