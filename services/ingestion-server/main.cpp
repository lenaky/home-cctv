#include "adapters/inbound/http/camera_api_controller.h"
#include "adapters/outbound/ipc/live_packet_feed_factory.h"
#include "adapters/outbound/pipeline/ffmpeg_ingest_pipeline.h"
#include "adapters/outbound/sqlite/sqlite_camera_repository.h"
#include "adapters/outbound/sqlite/sqlite_recording_repository.h"
#include "adapters/outbound/storage/local_storage_writer.h"
#include "application/list_cameras_usecase.h"
#include "application/register_camera_usecase.h"
#include "application/start_ingest_usecase.h"
#include "application/stop_ingest_usecase.h"
#include "domain/system_config.h"

#include <httplib.h>

int main() {
    // Placeholder default; not meant to be final.
    SystemConfig config{StorageLocation{StorageKind::Local, "/tmp/home-cctv/recordings"}};

    SqliteCameraRepository camera_repository("ingestion.db");
    SqliteRecordingRepository recording_repository("ingestion.db");
    LocalStorageWriter storage_writer;

    // Real per-camera lifecycle management is future work; one placeholder
    // feed is constructed here for the composition root to wire up.
    std::unique_ptr<ILivePacketFeed> live_feed = CreateLivePacketFeed(0);

    FfmpegIngestPipeline pipeline(recording_repository, storage_writer, *live_feed, config);

    RegisterCameraUseCase register_camera(camera_repository);
    ListCamerasUseCase list_cameras(camera_repository);
    StartIngestUseCase start_ingest(camera_repository, pipeline);
    StopIngestUseCase stop_ingest(pipeline);

    CameraApiController controller(register_camera, list_cameras, start_ingest, stop_ingest);

    httplib::Server server;
    controller.RegisterRoutes(server);

    server.listen("0.0.0.0", 8080);
    return 0;
}
