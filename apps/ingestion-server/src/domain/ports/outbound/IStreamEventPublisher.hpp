#pragma once
#include <string>

namespace homecctv::domain::ports {

class IStreamEventPublisher {
public:
    virtual ~IStreamEventPublisher() = default;
    virtual void onStreamStarted(const std::string& camera_id) = 0;
    virtual void onStreamStopped(const std::string& camera_id) = 0;
    virtual void onStreamError(const std::string& camera_id, const std::string& error) = 0;
    virtual void onStreamReconnecting(const std::string& camera_id) = 0;
};

}  // namespace homecctv::domain::ports
