#include "CameraService.hpp"

#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace homecctv::application {

static std::string generateId() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint32_t> dis;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    oss << std::setw(8) << dis(gen) << '-';
    oss << std::setw(4) << (dis(gen) & 0xFFFF) << '-';
    oss << std::setw(4) << ((dis(gen) & 0x0FFF) | 0x4000) << '-';
    oss << std::setw(4) << ((dis(gen) & 0x3FFF) | 0x8000) << '-';
    oss << std::setw(8) << dis(gen) << std::setw(4) << (dis(gen) & 0xFFFF);
    return oss.str();
}

CameraService::CameraService(std::shared_ptr<domain::ports::ICameraRepository> repo)
    : repo_(std::move(repo)) {}

Result<domain::Camera> CameraService::registerCamera(const domain::ports::RegisterCameraCmd& cmd) {
    if (cmd.name.empty()) return Result<domain::Camera>::Err("Camera name is required");
    if (cmd.rtsp_url.empty()) return Result<domain::Camera>::Err("RTSP URL is required");

    auto existing = repo_->findAll();
    if (existing.is_ok()) {
        for (const auto& cam : existing.value()) {
            if (cam.rtsp_url == cmd.rtsp_url)
                return Result<domain::Camera>::Err("A camera with this RTSP URL is already registered: " + cmd.rtsp_url);
        }
    }

    auto now = std::chrono::system_clock::now();
    domain::Camera cam;
    cam.id = generateId();
    cam.name = cmd.name;
    cam.rtsp_url = cmd.rtsp_url;
    cam.settings = cmd.settings;
    cam.status = domain::CameraStatus::Inactive;
    cam.created_at = now;
    cam.updated_at = now;

    return repo_->save(cam);
}

Result<domain::Camera> CameraService::getCamera(const std::string& id) {
    return repo_->findById(id);
}

Result<std::vector<domain::Camera>> CameraService::listCameras() {
    return repo_->findAll();
}

Result<domain::Camera> CameraService::updateCamera(const domain::ports::UpdateCameraCmd& cmd) {
    auto result = repo_->findById(cmd.id);
    if (result.is_err()) return result;

    auto cam = result.value();
    if (!cmd.name.empty()) cam.name = cmd.name;
    if (!cmd.rtsp_url.empty()) cam.rtsp_url = cmd.rtsp_url;
    cam.settings = cmd.settings;
    cam.updated_at = std::chrono::system_clock::now();

    return repo_->update(cam);
}

Result<void> CameraService::deleteCamera(const std::string& id) {
    return repo_->remove(id);
}

}  // namespace homecctv::application
