#pragma once
#include <httplib.h>

#include "application/create_webrtc_session_usecase.h"
#include "application/get_live_manifest_usecase.h"

class LiveApiController {
public:
    LiveApiController(GetLiveManifestUseCase& get_manifest,
                      CreateWebRtcSessionUseCase& create_webrtc_session);
    void RegisterRoutes(httplib::Server& server);

private:
    GetLiveManifestUseCase& get_manifest_;
    CreateWebRtcSessionUseCase& create_webrtc_session_;
};
