#include "adapters/outbound/hls/fs_hls_segment_reader.h"

FsHlsSegmentReader::FsHlsSegmentReader(std::string shared_fs_root)
    : shared_fs_root_(std::move(shared_fs_root)) {}

Result<std::vector<std::byte>> FsHlsSegmentReader::ReadManifest(CameraId camera_id) const {
    (void)camera_id;
    return Result<std::vector<std::byte>>::Err(
        Error{ErrorCode::NotImplemented, "FsHlsSegmentReader::ReadManifest not implemented"});
}

Result<std::vector<std::byte>>
FsHlsSegmentReader::ReadSegment(CameraId camera_id, std::string_view segment_name) const {
    (void)camera_id;
    (void)segment_name;
    return Result<std::vector<std::byte>>::Err(
        Error{ErrorCode::NotImplemented, "FsHlsSegmentReader::ReadSegment not implemented"});
}
