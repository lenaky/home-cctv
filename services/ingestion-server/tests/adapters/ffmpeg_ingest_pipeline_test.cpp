#include "adapters/outbound/pipeline/ffmpeg_ingest_pipeline.h"

#include <gtest/gtest.h>

namespace {

class FakeRecordingRepository : public IRecordingRepository {
public:
    Result<void> Save(const RecordingMetadata& metadata) override {
        (void)metadata;
        return Result<void>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
    Result<std::vector<RecordingMetadata>> FindByCameraAndRange(CameraId camera_id, Timestamp from,
                                                                Timestamp to) const override {
        (void)camera_id;
        (void)from;
        (void)to;
        return Result<std::vector<RecordingMetadata>>::Err(
            Error{ErrorCode::NotImplemented, "fake"});
    }
};

class FakeStorageWriter : public IRecordingStorageWriter {
public:
    Result<void> Write(const StorageLocation& location, std::string_view segment_name,
                       const std::vector<std::byte>& data) override {
        (void)location;
        (void)segment_name;
        (void)data;
        return Result<void>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
};

class FakeLivePacketFeed : public ILivePacketFeed {
public:
    Result<void> PublishPacket(CameraId camera_id, const EncodedPacket& packet) override {
        (void)camera_id;
        (void)packet;
        return Result<void>::Err(Error{ErrorCode::NotImplemented, "fake"});
    }
};

} // namespace

TEST(FfmpegIngestPipelineTest, StartReturnsNotImplemented) {
    FakeRecordingRepository recordings;
    FakeStorageWriter storage_writer;
    FakeLivePacketFeed live_feed;
    SystemConfig config{StorageLocation{StorageKind::Local, "/tmp/test"}};

    FfmpegIngestPipeline pipeline(recordings, storage_writer, live_feed, config);

    Camera camera{1, "front door", "rtsp://example"};
    Result<void> result = pipeline.Start(camera);

    EXPECT_TRUE(result.IsErr());
    EXPECT_EQ(result.GetError().code, ErrorCode::NotImplemented);
    EXPECT_FALSE(pipeline.IsRunning(1));
}
