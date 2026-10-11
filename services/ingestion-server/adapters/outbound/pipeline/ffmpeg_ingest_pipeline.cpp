#include "adapters/outbound/pipeline/ffmpeg_ingest_pipeline.h"

// No FFmpeg av* functions are called yet; this is structure only.

FfmpegIngestPipeline::FfmpegIngestPipeline(IRecordingRepository& recordings,
                                           IRecordingStorageWriter& storage_writer,
                                           ILivePacketFeed& live_feed, const SystemConfig& config)
    : recordings_(recordings), storage_writer_(storage_writer), live_feed_(live_feed),
      config_(config) {}

Result<void> FfmpegIngestPipeline::Start(const Camera& camera) {
    (void)camera;
    return Result<void>::Err(
        Error{ErrorCode::NotImplemented, "FfmpegIngestPipeline::Start not implemented"});
}

Result<void> FfmpegIngestPipeline::Stop(CameraId camera_id) {
    (void)camera_id;
    return Result<void>::Err(
        Error{ErrorCode::NotImplemented, "FfmpegIngestPipeline::Stop not implemented"});
}

bool FfmpegIngestPipeline::IsRunning(CameraId camera_id) const {
    (void)camera_id;
    return false;
}
