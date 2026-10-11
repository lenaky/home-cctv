#include "application/list_cameras_usecase.h"

ListCamerasUseCase::ListCamerasUseCase(ICameraRepository& cameras) : cameras_(cameras) {}

Result<std::vector<Camera>> ListCamerasUseCase::Execute() {
    return Result<std::vector<Camera>>::Err(
        Error{ErrorCode::NotImplemented, "ListCamerasUseCase::Execute not implemented"});
}
