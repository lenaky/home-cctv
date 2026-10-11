#pragma once
#include <vector>

#include "domain/recording_metadata.h"
#include "domain/result.h"
#include "domain/types.h"

// READ-ONLY query port, distinct from ingestion-server's write-side
// IRecordingRepository — egress never writes recording metadata.
class IRecordingQueryRepository {
public:
    virtual ~IRecordingQueryRepository() = default;
    virtual Result<std::vector<RecordingMetadata>>
    FindByCameraAndRange(CameraId camera_id, Timestamp from, Timestamp to) const = 0;
};
