#pragma once
#include "domain/camera.h"
#include "domain/result.h"
#include "domain/types.h"

class ICameraIngestPipeline {
public:
    virtual ~ICameraIngestPipeline() = default;
    virtual Result<void> Start(const Camera& camera) = 0;
    virtual Result<void> Stop(CameraId camera_id) = 0;
    virtual bool IsRunning(CameraId camera_id) const = 0;
};
