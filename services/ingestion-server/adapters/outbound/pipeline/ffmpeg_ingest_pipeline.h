#pragma once
#include "domain/ports/camera_ingest_pipeline.h"
#include "domain/ports/live_packet_feed.h"
#include "domain/ports/recording_repository.h"
#include "domain/ports/recording_storage_writer.h"
#include "domain/system_config.h"

class FfmpegIngestPipeline : public ICameraIngestPipeline {
public:
    FfmpegIngestPipeline(IRecordingRepository& recordings, IRecordingStorageWriter& storage_writer,
                         ILivePacketFeed& live_feed, const SystemConfig& config);

    Result<void> Start(const Camera& camera) override;
    Result<void> Stop(CameraId camera_id) override;
    bool IsRunning(CameraId camera_id) const override;

private:
    IRecordingRepository& recordings_;
    IRecordingStorageWriter& storage_writer_;
    ILivePacketFeed& live_feed_;
    const SystemConfig& config_;
};
