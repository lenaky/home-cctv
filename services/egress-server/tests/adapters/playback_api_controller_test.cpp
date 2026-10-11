#include <gtest/gtest.h>
#include <httplib.h>

#include "adapters/inbound/http/playback_api_controller.h"

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

TEST(PlaybackApiControllerTest, RegisterRoutesDoesNotThrow) {
    FakeRecordingQueryRepository fake_recordings;
    FakeRecordingStorageReader fake_storage;
    ListRecordingsUseCase list_recordings(fake_recordings);
    GetRecordingPlaybackUseCase get_playback(fake_recordings, fake_storage);
    PlaybackApiController controller(list_recordings, get_playback);

    httplib::Server server;
    EXPECT_NO_THROW(controller.RegisterRoutes(server));
}
