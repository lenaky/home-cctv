#include "adapters/outbound/sqlite/sqlite_recording_repository.h"

// Not opening a real sqlite3 connection yet; db_path_ is stored for future use.

SqliteRecordingRepository::SqliteRecordingRepository(std::string db_path)
    : db_path_(std::move(db_path)) {}

Result<void> SqliteRecordingRepository::Save(const RecordingMetadata& metadata) {
    (void)metadata;
    return Result<void>::Err(
        Error{ErrorCode::NotImplemented, "SqliteRecordingRepository::Save not implemented"});
}

Result<std::vector<RecordingMetadata>>
SqliteRecordingRepository::FindByCameraAndRange(CameraId camera_id, Timestamp from,
                                                Timestamp to) const {
    (void)camera_id;
    (void)from;
    (void)to;
    return Result<std::vector<RecordingMetadata>>::Err(
        Error{ErrorCode::NotImplemented,
              "SqliteRecordingRepository::FindByCameraAndRange not implemented"});
}
