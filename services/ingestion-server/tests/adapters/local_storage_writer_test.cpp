#include "adapters/outbound/storage/local_storage_writer.h"

#include <gtest/gtest.h>

TEST(LocalStorageWriterTest, WriteReturnsNotImplemented) {
    LocalStorageWriter writer;
    StorageLocation location{StorageKind::Local, "/tmp/test"};
    std::vector<std::byte> data;

    Result<void> result = writer.Write(location, "segment", data);

    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotImplemented);
}
