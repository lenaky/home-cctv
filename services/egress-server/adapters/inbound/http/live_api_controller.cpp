#include "adapters/inbound/http/live_api_controller.h"

#include <cstdlib>
#include <string>

LiveApiController::LiveApiController(GetLiveManifestUseCase& get_manifest,
                                     CreateWebRtcSessionUseCase& create_webrtc_session)
    : get_manifest_(get_manifest), create_webrtc_session_(create_webrtc_session) {}

namespace {

void WriteErrorResponse(httplib::Response& res, const Error& error) {
    if (error.code == ErrorCode::NotImplemented) {
        res.status = 501;
    } else if (error.code == ErrorCode::NotFound) {
        res.status = 404;
    } else if (error.code == ErrorCode::InvalidArgument) {
        res.status = 400;
    } else {
        res.status = 500;
    }
    res.set_content(error.message, "text/plain");
}

} // namespace

void LiveApiController::RegisterRoutes(httplib::Server& server) {
    server.Get(R"(/cameras/(\d+)/live/manifest)", [this](const httplib::Request& req,
                                                         httplib::Response& res) {
        CameraId camera_id =
            static_cast<CameraId>(std::strtoull(req.matches[1].str().c_str(), nullptr, 10));
        auto result = get_manifest_.Execute(camera_id);
        if (result.IsErr()) {
            WriteErrorResponse(res, result.GetError());
            return;
        }
        const auto& bytes = result.Value();
        res.set_content(reinterpret_cast<const char*>(bytes.data()), bytes.size(),
                        "application/vnd.apple.mpegurl");
    });

    server.Post(R"(/cameras/(\d+)/webrtc/session)", [this](const httplib::Request& req,
                                                           httplib::Response& res) {
        CameraId camera_id =
            static_cast<CameraId>(std::strtoull(req.matches[1].str().c_str(), nullptr, 10));
        auto result = create_webrtc_session_.Execute(camera_id, req.body);
        if (result.IsErr()) {
            WriteErrorResponse(res, result.GetError());
            return;
        }
        res.set_content(result.Value(), "application/sdp");
    });
}
