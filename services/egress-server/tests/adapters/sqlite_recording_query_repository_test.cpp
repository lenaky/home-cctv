#include <gtest/gtest.h>

#include "adapters/outbound/sqlite/sqlite_recording_query_repository.h"

TEST(SqliteRecordingQueryRepositoryTest, FindByCameraAndRangeNotImplemented) {
    SqliteRecordingQueryRepository repository("/tmp/home-cctv/home-cctv.db");
    auto result = repository.FindByCameraAndRange(1, Timestamp{}, Timestamp{});
    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotImplemented);
}
