#pragma once
#include <memory>

#include "domain/ports/inbound/ICameraService.hpp"
#include "domain/ports/outbound/ICameraRepository.hpp"

namespace homecctv::application {

class CameraService : public domain::ports::ICameraService {
public:
    explicit CameraService(std::shared_ptr<domain::ports::ICameraRepository> repo);

    Result<domain::Camera> registerCamera(const domain::ports::RegisterCameraCmd& cmd) override;
    Result<domain::Camera> getCamera(const std::string& id) override;
    Result<std::vector<domain::Camera>> listCameras() override;
    Result<domain::Camera> updateCamera(const domain::ports::UpdateCameraCmd& cmd) override;
    Result<void> deleteCamera(const std::string& id) override;

private:
    std::shared_ptr<domain::ports::ICameraRepository> repo_;
};

}  // namespace homecctv::application
