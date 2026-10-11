#include "adapters/outbound/storage/local_storage_reader.h"

Result<std::vector<std::byte>> LocalStorageReader::Read(const StorageLocation& location,
                                                        std::string_view segment_name) const {
    (void)location;
    (void)segment_name;
    return Result<std::vector<std::byte>>::Err(
        Error{ErrorCode::NotImplemented, "LocalStorageReader::Read not implemented"});
}
