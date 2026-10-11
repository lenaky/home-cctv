#include <gtest/gtest.h>

#include "adapters/outbound/hls/fs_hls_segment_reader.h"

TEST(FsHlsSegmentReaderTest, ReadManifestNotImplemented) {
    FsHlsSegmentReader reader("/tmp/home-cctv/hls");
    auto result = reader.ReadManifest(1);
    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotImplemented);
}

TEST(FsHlsSegmentReaderTest, ReadSegmentNotImplemented) {
    FsHlsSegmentReader reader("/tmp/home-cctv/hls");
    auto result = reader.ReadSegment(1, "segment.ts");
    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotImplemented);
}
