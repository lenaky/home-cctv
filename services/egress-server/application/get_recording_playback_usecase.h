#pragma once
#include <cstddef>
#include <string_view>
#include <vector>

#include "domain/ports/recording_query_repository.h"
#include "domain/ports/recording_storage_reader.h"
#include "domain/result.h"
#include "domain/types.h"

class GetRecordingPlaybackUseCase {
public:
    GetRecordingPlaybackUseCase(IRecordingQueryRepository& recordings,
                                IRecordingStorageReader& storage);
    Result<std::vector<std::byte>> Execute(CameraId camera_id, Timestamp at,
                                           std::string_view segment_name);

private:
    IRecordingQueryRepository& recordings_;
    IRecordingStorageReader& storage_;
};
