#pragma once
#include <string>
#include <vector>

#include "domain/ports/recording_query_repository.h"

class SqliteRecordingQueryRepository : public IRecordingQueryRepository {
public:
    explicit SqliteRecordingQueryRepository(std::string db_path);
    Result<std::vector<RecordingMetadata>> FindByCameraAndRange(CameraId camera_id, Timestamp from,
                                                                Timestamp to) const override;

private:
    std::string db_path_;
};
