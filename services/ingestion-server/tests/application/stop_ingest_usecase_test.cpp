#include "application/stop_ingest_usecase.h"

#include <gtest/gtest.h>

namespace {

class FakeCameraIngestPipeline : public ICameraIngestPipeline {
public:
    Result<void> Start(const Camera& camera) override {
        (void)camera;
        return Result<void>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
    Result<void> Stop(CameraId camera_id) override {
        (void)camera_id;
        return Result<void>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
    bool IsRunning(CameraId camera_id) const override {
        (void)camera_id;
        return false;
    }
};

} // namespace

TEST(StopIngestUseCaseTest, ExecuteReturnsNotImplemented) {
    FakeCameraIngestPipeline pipeline;
    StopIngestUseCase usecase(pipeline);

    Result<void> result = usecase.Execute(1);

    EXPECT_TRUE(result.IsErr());
}
