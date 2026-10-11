#pragma once
#include "domain/ports/camera_ingest_pipeline.h"
#include "domain/result.h"
#include "domain/types.h"

class StopIngestUseCase {
public:
    explicit StopIngestUseCase(ICameraIngestPipeline& pipeline);
    Result<void> Execute(CameraId camera_id);

private:
    ICameraIngestPipeline& pipeline_;
};
