#pragma once
#include <httplib.h>

#include "application/get_recording_playback_usecase.h"
#include "application/list_recordings_usecase.h"

class PlaybackApiController {
public:
    PlaybackApiController(ListRecordingsUseCase& list_recordings,
                          GetRecordingPlaybackUseCase& get_playback);
    void RegisterRoutes(httplib::Server& server);

private:
    ListRecordingsUseCase& list_recordings_;
    GetRecordingPlaybackUseCase& get_playback_;
};
