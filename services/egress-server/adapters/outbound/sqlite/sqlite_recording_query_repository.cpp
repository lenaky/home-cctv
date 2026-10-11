#include "adapters/outbound/sqlite/sqlite_recording_query_repository.h"

// Reads the same sqlite db file that ingestion-server writes to (WAL mode,
// each service opens its own connection — no shared code, just an
// agreed-upon file path passed via config/args). Real sqlite3_* calls not
// implemented here.

SqliteRecordingQueryRepository::SqliteRecordingQueryRepository(std::string db_path)
    : db_path_(std::move(db_path)) {}

Result<std::vector<RecordingMetadata>>
SqliteRecordingQueryRepository::FindByCameraAndRange(CameraId camera_id, Timestamp from,
                                                     Timestamp to) const {
    (void)camera_id;
    (void)from;
    (void)to;
    return Result<std::vector<RecordingMetadata>>::Err(
        Error{ErrorCode::NotImplemented,
              "SqliteRecordingQueryRepository::FindByCameraAndRange not implemented"});
}
