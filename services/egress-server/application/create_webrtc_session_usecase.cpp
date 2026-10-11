#include "application/create_webrtc_session_usecase.h"

CreateWebRtcSessionUseCase::CreateWebRtcSessionUseCase(IWebRtcEgressPipeline& pipeline)
    : pipeline_(pipeline) {}

Result<std::string> CreateWebRtcSessionUseCase::Execute(CameraId camera_id,
                                                        std::string_view sdp_offer) {
    (void)camera_id;
    (void)sdp_offer;
    return Result<std::string>::Err(
        Error{ErrorCode::NotImplemented, "CreateWebRtcSessionUseCase::Execute not implemented"});
}
