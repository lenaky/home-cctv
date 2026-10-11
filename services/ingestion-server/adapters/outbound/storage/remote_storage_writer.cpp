#include "adapters/outbound/storage/remote_storage_writer.h"

Result<void> RemoteStorageWriter::Write(const StorageLocation& location,
                                        std::string_view segment_name,
                                        const std::vector<std::byte>& data) {
    (void)location;
    (void)segment_name;
    (void)data;
    return Result<void>::Err(
        Error{ErrorCode::NotImplemented, "RemoteStorageWriter::Write not implemented"});
}
