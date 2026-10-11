#include <gtest/gtest.h>

#include "application/get_recording_playback_usecase.h"

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

class FakeRecordingStorageReader : public IRecordingStorageReader {
public:
    Result<std::vector<std::byte>> Read(const StorageLocation& location,
                                        std::string_view segment_name) const override {
        (void)location;
        (void)segment_name;
        return Result<std::vector<std::byte>>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
};

} // namespace

TEST(GetRecordingPlaybackUseCaseTest, ExecuteReturnsErr) {
    FakeRecordingQueryRepository fake_recordings;
    FakeRecordingStorageReader fake_storage;
    GetRecordingPlaybackUseCase use_case(fake_recordings, fake_storage);
    auto result = use_case.Execute(1, Timestamp{}, "segment.ts");
    EXPECT_TRUE(result.IsErr());
}
