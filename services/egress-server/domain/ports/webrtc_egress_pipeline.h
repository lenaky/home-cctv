#pragma once
#include <string>
#include <string_view>

#include "domain/result.h"
#include "domain/types.h"

class IWebRtcEgressPipeline {
public:
    virtual ~IWebRtcEgressPipeline() = default;
    virtual Result<std::string> CreateSession(CameraId camera_id,
                                              std::string_view sdp_offer) = 0; // returns sdp answer
    virtual Result<void> CloseSession(std::string_view session_id) = 0;
};
