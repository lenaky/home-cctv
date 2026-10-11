#include "application/register_camera_usecase.h"

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

} // namespace

TEST(RegisterCameraUseCaseTest, ExecuteReturnsNotImplemented) {
    FakeCameraRepository cameras;
    RegisterCameraUseCase usecase(cameras);

    Camera camera{1, "front door", "rtsp://example"};
    Result<void> result = usecase.Execute(camera);

    EXPECT_TRUE(result.IsErr());
}
