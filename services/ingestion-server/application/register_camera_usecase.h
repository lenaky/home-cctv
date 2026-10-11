#pragma once
#include "domain/camera.h"
#include "domain/ports/camera_repository.h"
#include "domain/result.h"

class RegisterCameraUseCase {
public:
    explicit RegisterCameraUseCase(ICameraRepository& cameras);
    Result<void> Execute(const Camera& camera);

private:
    ICameraRepository& cameras_;
};
