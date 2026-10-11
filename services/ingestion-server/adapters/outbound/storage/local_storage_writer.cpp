#include "adapters/outbound/storage/local_storage_writer.h"

Result<void> LocalStorageWriter::Write(const StorageLocation& location,
                                       std::string_view segment_name,
                                       const std::vector<std::byte>& data) {
    (void)location;
    (void)segment_name;
    (void)data;
    return Result<void>::Err(
        Error{ErrorCode::NotImplemented, "LocalStorageWriter::Write not implemented"});
}
