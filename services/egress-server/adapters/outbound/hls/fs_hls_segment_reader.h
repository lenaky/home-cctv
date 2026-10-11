#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "domain/ports/hls_segment_reader.h"

class FsHlsSegmentReader : public IHlsSegmentReader {
public:
    explicit FsHlsSegmentReader(std::string shared_fs_root);
    Result<std::vector<std::byte>> ReadManifest(CameraId camera_id) const override;
    Result<std::vector<std::byte>> ReadSegment(CameraId camera_id,
                                               std::string_view segment_name) const override;

private:
    std::string shared_fs_root_;
};
