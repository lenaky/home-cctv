#pragma once
#include <cstddef>
#include <string_view>
#include <vector>

#include "domain/ports/recording_storage_reader.h"

class LocalStorageReader : public IRecordingStorageReader {
public:
    Result<std::vector<std::byte>> Read(const StorageLocation& location,
                                        std::string_view segment_name) const override;
};
