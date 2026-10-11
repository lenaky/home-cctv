#include "application/get_live_manifest_usecase.h"

GetLiveManifestUseCase::GetLiveManifestUseCase(IHlsSegmentReader& reader) : reader_(reader) {}

Result<std::vector<std::byte>> GetLiveManifestUseCase::Execute(CameraId camera_id) {
    (void)camera_id;
    return Result<std::vector<std::byte>>::Err(
        Error{ErrorCode::NotImplemented, "GetLiveManifestUseCase::Execute not implemented"});
}
