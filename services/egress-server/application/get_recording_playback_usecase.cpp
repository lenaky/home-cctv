#include "application/get_recording_playback_usecase.h"

GetRecordingPlaybackUseCase::GetRecordingPlaybackUseCase(IRecordingQueryRepository& recordings,
                                                         IRecordingStorageReader& storage)
    : recordings_(recordings), storage_(storage) {}

Result<std::vector<std::byte>> GetRecordingPlaybackUseCase::Execute(CameraId camera_id,
                                                                    Timestamp at,
                                                                    std::string_view segment_name) {
    (void)camera_id;
    (void)at;
    (void)segment_name;
    return Result<std::vector<std::byte>>::Err(
        Error{ErrorCode::NotImplemented, "GetRecordingPlaybackUseCase::Execute not implemented"});
}
