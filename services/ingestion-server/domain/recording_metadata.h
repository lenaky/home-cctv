#pragma once
#include "domain/types.h"

struct RecordingMetadata {
    CameraId camera_id;
    Timestamp start_time;
    Timestamp end_time;
    StorageLocation location;
};
