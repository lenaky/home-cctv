#pragma once
#include "domain/ports/camera_repository.h"
#include <string>

class SqliteCameraRepository : public ICameraRepository {
public:
    explicit SqliteCameraRepository(std::string db_path);

    Result<Camera> FindById(CameraId id) const override;
    Result<std::vector<Camera>> FindAll() const override;
    Result<void> Save(const Camera& camera) override;
    Result<void> Remove(CameraId id) override;

private:
    std::string db_path_;
};
