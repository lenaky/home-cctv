#pragma once
#include <cstddef>
#include <string_view>
#include <vector>

#include "domain/result.h"
#include "domain/types.h"

class IHlsSegmentReader {
public:
    virtual ~IHlsSegmentReader() = default;
    virtual Result<std::vector<std::byte>> ReadManifest(CameraId camera_id) const = 0;
    virtual Result<std::vector<std::byte>> ReadSegment(CameraId camera_id,
                                                       std::string_view segment_name) const = 0;
};
