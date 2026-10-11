#include "adapters/outbound/sqlite/sqlite_camera_repository.h"

// Not opening a real sqlite3 connection yet; db_path_ is stored for future use.

SqliteCameraRepository::SqliteCameraRepository(std::string db_path)
    : db_path_(std::move(db_path)) {}

Result<Camera> SqliteCameraRepository::FindById(CameraId id) const {
    (void)id;
    return Result<Camera>::Err(
        Error{ErrorCode::NotImplemented, "SqliteCameraRepository::FindById not implemented"});
}

Result<std::vector<Camera>> SqliteCameraRepository::FindAll() const {
    return Result<std::vector<Camera>>::Err(
        Error{ErrorCode::NotImplemented, "SqliteCameraRepository::FindAll not implemented"});
}

Result<void> SqliteCameraRepository::Save(const Camera& camera) {
    (void)camera;
    return Result<void>::Err(
        Error{ErrorCode::NotImplemented, "SqliteCameraRepository::Save not implemented"});
}

Result<void> SqliteCameraRepository::Remove(CameraId id) {
    (void)id;
    return Result<void>::Err(
        Error{ErrorCode::NotImplemented, "SqliteCameraRepository::Remove not implemented"});
}
