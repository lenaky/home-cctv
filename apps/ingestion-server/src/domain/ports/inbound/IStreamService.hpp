#pragma once
#include <string>

#include "core/Result.hpp"
#include "../../entities/Stream.hpp"

namespace homecctv::domain::ports {

class IStreamService {
public:
    virtual ~IStreamService() = default;
    virtual Result<StreamInfo> startStream(const std::string& camera_id) = 0;
    virtual Result<void> stopStream(const std::string& camera_id) = 0;
    virtual Result<StreamInfo> getStreamStatus(const std::string& camera_id) = 0;
};

}  // namespace homecctv::domain::ports
