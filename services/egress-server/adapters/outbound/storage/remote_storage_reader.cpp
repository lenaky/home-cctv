#include "adapters/outbound/storage/remote_storage_reader.h"

Result<std::vector<std::byte>> RemoteStorageReader::Read(const StorageLocation& location,
                                                         std::string_view segment_name) const {
    (void)location;
    (void)segment_name;
    return Result<std::vector<std::byte>>::Err(
        Error{ErrorCode::NotImplemented, "RemoteStorageReader::Read not implemented"});
}
