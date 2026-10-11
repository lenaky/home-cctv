#include <gtest/gtest.h>
#include <string>

#include "domain/result.h"

TEST(ResultTest, OkHoldsValue) {
    auto result = Result<int>::Ok(42);
    EXPECT_TRUE(result.IsOk());
    EXPECT_FALSE(result.IsErr());
    EXPECT_EQ(result.Value(), 42);
}

TEST(ResultTest, ErrHoldsError) {
    auto result = Result<int>::Err(Error{ErrorCode::NotFound, "missing"});
    EXPECT_TRUE(result.IsErr());
    EXPECT_FALSE(result.IsOk());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotFound);
    EXPECT_EQ(result.GetError().message, "missing");
}

TEST(ResultTest, MutableValueAccessor) {
    auto result = Result<std::string>::Ok("hello");
    result.Value() += " world";
    EXPECT_EQ(result.Value(), "hello world");
}

TEST(ResultVoidTest, OkIsOk) {
    auto result = Result<void>::Ok();
    EXPECT_TRUE(result.IsOk());
    EXPECT_FALSE(result.IsErr());
}

TEST(ResultVoidTest, ErrHoldsError) {
    auto result = Result<void>::Err(Error{ErrorCode::IoError, "disk full"});
    EXPECT_TRUE(result.IsErr());
    EXPECT_FALSE(result.IsOk());
    EXPECT_EQ(result.GetError().code, ErrorCode::IoError);
    EXPECT_EQ(result.GetError().message, "disk full");
}
