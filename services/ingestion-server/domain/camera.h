#pragma once
#include "domain/types.h"
#include <string>

// Note: deliberately no storage field here — recording storage location is a
// system-wide setting, see domain/system_config.h. Do not add it back.
struct Camera {
    CameraId id;
    std::string name;
    std::string rtsp_url;
};
