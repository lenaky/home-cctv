#pragma once
#include "domain/recording_metadata.h"
#include "domain/result.h"
#include "domain/types.h"
#include <vector>

class IRecordingRepository {
public:
    virtual ~IRecordingRepository() = default;
    virtual Result<void> Save(const RecordingMetadata& metadata) = 0;
    virtual Result<std::vector<RecordingMetadata>>
    FindByCameraAndRange(CameraId camera_id, Timestamp from, Timestamp to) const = 0;
};
