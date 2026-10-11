#include "application/start_ingest_usecase.h"

StartIngestUseCase::StartIngestUseCase(ICameraRepository& cameras, ICameraIngestPipeline& pipeline)
    : cameras_(cameras), pipeline_(pipeline) {}

Result<void> StartIngestUseCase::Execute(CameraId camera_id) {
    (void)camera_id;
    return Result<void>::Err(
        Error{ErrorCode::NotImplemented, "StartIngestUseCase::Execute not implemented"});
}
