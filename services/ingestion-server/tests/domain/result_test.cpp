#include "domain/result.h"

#include <gtest/gtest.h>

TEST(ResultTest, OkHoldsValue) {
    Result<int> result = Result<int>::Ok(42);
    EXPECT_TRUE(result.IsOk());
    EXPECT_FALSE(result.IsErr());
    EXPECT_EQ(result.Value(), 42);
}

TEST(ResultTest, ErrHoldsError) {
    Result<int> result = Result<int>::Err(Error{ErrorCode::NotFound, "missing"});
    EXPECT_TRUE(result.IsErr());
    EXPECT_FALSE(result.IsOk());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotFound);
    EXPECT_EQ(result.GetError().message, "missing");
}

TEST(ResultVoidTest, OkIsOk) {
    Result<void> result = Result<void>::Ok();
    EXPECT_TRUE(result.IsOk());
    EXPECT_FALSE(result.IsErr());
}

TEST(ResultVoidTest, ErrHoldsError) {
    Result<void> result = Result<void>::Err(Error{ErrorCode::IoError, "disk full"});
    EXPECT_TRUE(result.IsErr());
    EXPECT_FALSE(result.IsOk());
    EXPECT_EQ(result.GetError().code, ErrorCode::IoError);
    EXPECT_EQ(result.GetError().message, "disk full");
}
