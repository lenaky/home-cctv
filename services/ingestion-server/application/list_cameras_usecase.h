#pragma once
#include "domain/camera.h"
#include "domain/ports/camera_repository.h"
#include "domain/result.h"
#include <vector>

class ListCamerasUseCase {
public:
    explicit ListCamerasUseCase(ICameraRepository& cameras);
    Result<std::vector<Camera>> Execute();

private:
    ICameraRepository& cameras_;
};
