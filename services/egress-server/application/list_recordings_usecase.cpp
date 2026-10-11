#include "application/list_recordings_usecase.h"

ListRecordingsUseCase::ListRecordingsUseCase(IRecordingQueryRepository& recordings)
    : recordings_(recordings) {}

Result<std::vector<RecordingMetadata>>
ListRecordingsUseCase::Execute(CameraId camera_id, Timestamp from, Timestamp to) {
    (void)camera_id;
    (void)from;
    (void)to;
    return Result<std::vector<RecordingMetadata>>::Err(
        Error{ErrorCode::NotImplemented, "ListRecordingsUseCase::Execute not implemented"});
}
