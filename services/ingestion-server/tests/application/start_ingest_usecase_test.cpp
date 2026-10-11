#include "application/start_ingest_usecase.h"

#include <gtest/gtest.h>

namespace {

class FakeCameraRepository : public ICameraRepository {
public:
    Result<Camera> FindById(CameraId id) const override {
        (void)id;
        return Result<Camera>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
    Result<std::vector<Camera>> FindAll() const override {
        return Result<std::vector<Camera>>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
    Result<void> Save(const Camera& camera) override {
        (void)camera;
        return Result<void>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
    Result<void> Remove(CameraId id) override {
        (void)id;
        return Result<void>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
};

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

TEST(StartIngestUseCaseTest, ExecuteReturnsNotImplemented) {
    FakeCameraRepository cameras;
    FakeCameraIngestPipeline pipeline;
    StartIngestUseCase usecase(cameras, pipeline);

    Result<void> result = usecase.Execute(1);

    EXPECT_TRUE(result.IsErr());
}
