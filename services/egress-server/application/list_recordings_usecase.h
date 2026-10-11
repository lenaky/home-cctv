#pragma once
#include <vector>

#include "domain/ports/recording_query_repository.h"
#include "domain/recording_metadata.h"
#include "domain/result.h"
#include "domain/types.h"

class ListRecordingsUseCase {
public:
    explicit ListRecordingsUseCase(IRecordingQueryRepository& recordings);
    Result<std::vector<RecordingMetadata>> Execute(CameraId camera_id, Timestamp from,
                                                   Timestamp to);

private:
    IRecordingQueryRepository& recordings_;
};
