#include <httplib.h>

#include "application/create_webrtc_session_usecase.h"
#include "application/get_live_manifest_usecase.h"
#include "application/get_recording_playback_usecase.h"
#include "application/list_recordings_usecase.h"

#include "adapters/inbound/http/live_api_controller.h"
#include "adapters/inbound/http/playback_api_controller.h"

#include "adapters/outbound/hls/fs_hls_segment_reader.h"
#include "adapters/outbound/sqlite/sqlite_recording_query_repository.h"
#include "adapters/outbound/storage/local_storage_reader.h"
#include "adapters/outbound/webrtc/ffmpeg_webrtc_sender.h"

int main() {
    // Composition root: wire concrete adapters to ports, ports to use cases,
    // use cases to inbound controllers. No business logic here.
    FsHlsSegmentReader hls_segment_reader("/tmp/home-cctv/hls");
    FfmpegWebRtcSender webrtc_sender;
    LocalStorageReader local_storage_reader;
    SqliteRecordingQueryRepository recording_query_repository("/tmp/home-cctv/home-cctv.db");

    GetLiveManifestUseCase get_live_manifest(hls_segment_reader);
    CreateWebRtcSessionUseCase create_webrtc_session(webrtc_sender);
    ListRecordingsUseCase list_recordings(recording_query_repository);
    GetRecordingPlaybackUseCase get_recording_playback(recording_query_repository,
                                                       local_storage_reader);

    LiveApiController live_api_controller(get_live_manifest, create_webrtc_session);
    PlaybackApiController playback_api_controller(list_recordings, get_recording_playback);

    httplib::Server server;
    live_api_controller.RegisterRoutes(server);
    playback_api_controller.RegisterRoutes(server);

    server.listen("0.0.0.0", 8090);
    return 0;
}
