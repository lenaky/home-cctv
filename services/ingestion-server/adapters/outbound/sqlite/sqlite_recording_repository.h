#pragma once
#include "domain/ports/recording_repository.h"
#include <string>

class SqliteRecordingRepository : public IRecordingRepository {
public:
    explicit SqliteRecordingRepository(std::string db_path);

    Result<void> Save(const RecordingMetadata& metadata) override;
    Result<std::vector<RecordingMetadata>> FindByCameraAndRange(CameraId camera_id, Timestamp from,
                                                                Timestamp to) const override;

private:
    std::string db_path_;
};
