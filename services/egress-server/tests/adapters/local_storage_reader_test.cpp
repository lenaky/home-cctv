#include <gtest/gtest.h>

#include "adapters/outbound/storage/local_storage_reader.h"

TEST(LocalStorageReaderTest, ReadNotImplemented) {
    LocalStorageReader reader;
    StorageLocation location{StorageKind::Local, "/tmp/home-cctv/recordings"};
    auto result = reader.Read(location, "segment.ts");
    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotImplemented);
}
