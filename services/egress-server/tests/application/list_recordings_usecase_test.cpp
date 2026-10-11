#include <gtest/gtest.h>

#include "application/list_recordings_usecase.h"

namespace {

class FakeRecordingQueryRepository : public IRecordingQueryRepository {
public:
    Result<std::vector<RecordingMetadata>> FindByCameraAndRange(CameraId camera_id, Timestamp from,
                                                                Timestamp to) const override {
        (void)camera_id;
        (void)from;
        (void)to;
        return Result<std::vector<RecordingMetadata>>::Err(
            Error{ErrorCode::NotImplemented, "fake"});
    }
};

} // namespace

TEST(ListRecordingsUseCaseTest, ExecuteReturnsErr) {
    FakeRecordingQueryRepository fake_recordings;
    ListRecordingsUseCase use_case(fake_recordings);
    auto result = use_case.Execute(1, Timestamp{}, Timestamp{});
    EXPECT_TRUE(result.IsErr());
}
