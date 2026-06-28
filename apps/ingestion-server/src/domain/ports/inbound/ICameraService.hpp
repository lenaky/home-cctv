#pragma once
#include <string>
#include <vector>

#include "core/Result.hpp"
#include "../../entities/Camera.hpp"

namespace homecctv::domain::ports {

struct RegisterCameraCmd {
    std::string name;
    std::string rtsp_url;
    CameraSettings settings;
};

struct UpdateCameraCmd {
    std::string id;
    std::string name;
    std::string rtsp_url;
    CameraSettings settings;
};

class ICameraService {
public:
    virtual ~ICameraService() = default;
    virtual Result<Camera> registerCamera(const RegisterCameraCmd& cmd) = 0;
    virtual Result<Camera> getCamera(const std::string& id) = 0;
    virtual Result<std::vector<Camera>> listCameras() = 0;
    virtual Result<Camera> updateCamera(const UpdateCameraCmd& cmd) = 0;
    virtual Result<void> deleteCamera(const std::string& id) = 0;
};

}  // namespace homecctv::domain::ports
