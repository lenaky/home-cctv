#pragma once
#include "domain/ports/camera_ingest_pipeline.h"
#include "domain/ports/camera_repository.h"
#include "domain/result.h"
#include "domain/types.h"

class StartIngestUseCase {
public:
    StartIngestUseCase(ICameraRepository& cameras, ICameraIngestPipeline& pipeline);
    Result<void> Execute(CameraId camera_id);

private:
    ICameraRepository& cameras_;
    ICameraIngestPipeline& pipeline_;
};
