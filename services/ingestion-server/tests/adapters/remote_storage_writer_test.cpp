#include "adapters/outbound/storage/remote_storage_writer.h"

#include <gtest/gtest.h>

TEST(RemoteStorageWriterTest, WriteReturnsNotImplemented) {
    RemoteStorageWriter writer;
    StorageLocation location{StorageKind::Remote, "s3://bucket/path"};
    std::vector<std::byte> data;

    Result<void> result = writer.Write(location, "segment", data);

    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotImplemented);
}
