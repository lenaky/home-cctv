#include "adapters/outbound/sqlite/sqlite_recording_repository.h"

#include <gtest/gtest.h>

TEST(SqliteRecordingRepositoryTest, SaveReturnsNotImplemented) {
    SqliteRecordingRepository repository("test.db");

    RecordingMetadata metadata{};
    Result<void> result = repository.Save(metadata);

    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotImplemented);
}
