#include "application/stop_ingest_usecase.h"

StopIngestUseCase::StopIngestUseCase(ICameraIngestPipeline& pipeline) : pipeline_(pipeline) {}

Result<void> StopIngestUseCase::Execute(CameraId camera_id) {
    (void)camera_id;
    return Result<void>::Err(
        Error{ErrorCode::NotImplemented, "StopIngestUseCase::Execute not implemented"});
}
