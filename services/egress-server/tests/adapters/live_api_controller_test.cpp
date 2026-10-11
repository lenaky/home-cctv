#include <gtest/gtest.h>
#include <httplib.h>

#include "adapters/inbound/http/live_api_controller.h"

namespace {

class FakeHlsSegmentReader : public IHlsSegmentReader {
public:
    Result<std::vector<std::byte>> ReadManifest(CameraId camera_id) const override {
        (void)camera_id;
        return Result<std::vector<std::byte>>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
    Result<std::vector<std::byte>> ReadSegment(CameraId camera_id,
                                               std::string_view segment_name) const override {
        (void)camera_id;
        (void)segment_name;
        return Result<std::vector<std::byte>>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
};

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

TEST(LiveApiControllerTest, RegisterRoutesDoesNotThrow) {
    FakeHlsSegmentReader fake_reader;
    FakeWebRtcEgressPipeline fake_pipeline;
    GetLiveManifestUseCase get_manifest(fake_reader);
    CreateWebRtcSessionUseCase create_webrtc_session(fake_pipeline);
    LiveApiController controller(get_manifest, create_webrtc_session);

    httplib::Server server;
    EXPECT_NO_THROW(controller.RegisterRoutes(server));
}
