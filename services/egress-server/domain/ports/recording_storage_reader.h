#pragma once
#include <cstddef>
#include <string_view>
#include <vector>

#include "domain/result.h"
#include "domain/types.h"

class IRecordingStorageReader {
public:
    virtual ~IRecordingStorageReader() = default;
    virtual Result<std::vector<std::byte>> Read(const StorageLocation& location,
                                                std::string_view segment_name) const = 0;
};
