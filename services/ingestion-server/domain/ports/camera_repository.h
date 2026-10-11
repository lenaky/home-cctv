#pragma once
#include "domain/camera.h"
#include "domain/result.h"
#include <vector>

class ICameraRepository {
public:
    virtual ~ICameraRepository() = default;
    virtual Result<Camera> FindById(CameraId id) const = 0;
    virtual Result<std::vector<Camera>> FindAll() const = 0;
    virtual Result<void> Save(const Camera& camera) = 0;
    virtual Result<void> Remove(CameraId id) = 0;
};
