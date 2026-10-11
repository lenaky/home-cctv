#include <gtest/gtest.h>

#include "adapters/outbound/webrtc/ffmpeg_webrtc_sender.h"

TEST(FfmpegWebRtcSenderTest, CreateSessionNotImplemented) {
    FfmpegWebRtcSender sender;
    auto result = sender.CreateSession(1, "offer");
    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotImplemented);
}

TEST(FfmpegWebRtcSenderTest, CloseSessionNotImplemented) {
    FfmpegWebRtcSender sender;
    auto result = sender.CloseSession("session-id");
    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotImplemented);
}
