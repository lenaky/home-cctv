#include <gtest/gtest.h>

#include "application/get_live_manifest_usecase.h"

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

} // namespace

TEST(GetLiveManifestUseCaseTest, ExecuteReturnsErr) {
    FakeHlsSegmentReader fake_reader;
    GetLiveManifestUseCase use_case(fake_reader);
    auto result = use_case.Execute(1);
    EXPECT_TRUE(result.IsErr());
}
