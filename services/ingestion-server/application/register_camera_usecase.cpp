#include "application/register_camera_usecase.h"

RegisterCameraUseCase::RegisterCameraUseCase(ICameraRepository& cameras) : cameras_(cameras) {}

Result<void> RegisterCameraUseCase::Execute(const Camera& camera) {
    (void)camera;
    return Result<void>::Err(
        Error{ErrorCode::NotImplemented, "RegisterCameraUseCase::Execute not implemented"});
}
