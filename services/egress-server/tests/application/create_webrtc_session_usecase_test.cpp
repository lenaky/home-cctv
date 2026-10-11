#include <gtest/gtest.h>

#include "application/create_webrtc_session_usecase.h"

namespace {

class FakeWebRtcEgressPipeline : public IWebRtcEgressPipeline {
public:
    Result<std::string> CreateSession(CameraId camera_id, std::string_view sdp_offer) override {
        (void)camera_id;
        (void)sdp_offer;
        return Result<std::string>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
    Result<void> CloseSession(std::string_view session_id) override {
        (void)session_id;
        return Result<void>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
};

} // namespace

TEST(CreateWebRtcSessionUseCaseTest, ExecuteReturnsErr) {
    FakeWebRtcEgressPipeline fake_pipeline;
    CreateWebRtcSessionUseCase use_case(fake_pipeline);
    auto result = use_case.Execute(1, "offer");
    EXPECT_TRUE(result.IsErr());
}
