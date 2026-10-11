#include <gtest/gtest.h>

#include "adapters/outbound/storage/remote_storage_reader.h"

TEST(RemoteStorageReaderTest, ReadNotImplemented) {
    RemoteStorageReader reader;
    StorageLocation location{StorageKind::Remote, "s3://bucket/key"};
    auto result = reader.Read(location, "segment.ts");
    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotImplemented);
}
