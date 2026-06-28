#pragma once
#include <string>
#include <vector>

#include "core/Result.hpp"
#include "../../entities/Camera.hpp"

namespace homecctv::domain::ports {

class ICameraRepository {
public:
    virtual ~ICameraRepository() = default;
    virtual Result<Camera> findById(const std::string& id) = 0;
    virtual Result<std::vector<Camera>> findAll() = 0;
    virtual Result<Camera> save(const Camera& camera) = 0;
    virtual Result<Camera> update(const Camera& camera) = 0;
    virtual Result<void> remove(const std::string& id) = 0;
    virtual Result<void> updateStatus(const std::string& id, CameraStatus status,
                                       const std::optional<std::string>& error = std::nullopt) = 0;
};

}  // namespace homecctv::domain::ports
