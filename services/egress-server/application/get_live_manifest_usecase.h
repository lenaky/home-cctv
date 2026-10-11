#pragma once
#include <cstddef>
#include <vector>

#include "domain/ports/hls_segment_reader.h"
#include "domain/result.h"
#include "domain/types.h"

class GetLiveManifestUseCase {
public:
    explicit GetLiveManifestUseCase(IHlsSegmentReader& reader);
    Result<std::vector<std::byte>> Execute(CameraId camera_id);

private:
    IHlsSegmentReader& reader_;
};
