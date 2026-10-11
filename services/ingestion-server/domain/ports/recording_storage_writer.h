#pragma once
#include "domain/result.h"
#include "domain/types.h"
#include <cstddef>
#include <string_view>
#include <vector>

class IRecordingStorageWriter {
public:
    virtual ~IRecordingStorageWriter() = default;
    virtual Result<void> Write(const StorageLocation& location, std::string_view segment_name,
                               const std::vector<std::byte>& data) = 0;
};
