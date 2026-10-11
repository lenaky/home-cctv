#pragma once
#include "domain/ports/recording_storage_writer.h"

class LocalStorageWriter : public IRecordingStorageWriter {
public:
    Result<void> Write(const StorageLocation& location, std::string_view segment_name,
                       const std::vector<std::byte>& data) override;
};
