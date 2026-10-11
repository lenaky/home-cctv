#include "adapters/outbound/sqlite/sqlite_camera_repository.h"

#include <gtest/gtest.h>

TEST(SqliteCameraRepositoryTest, FindByIdReturnsNotImplemented) {
    SqliteCameraRepository repository("test.db");

    Result<Camera> result = repository.FindById(1);

    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotImplemented);
}
