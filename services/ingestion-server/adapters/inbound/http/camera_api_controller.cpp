#include "adapters/inbound/http/camera_api_controller.h"

#include "domain/camera.h"
#include "domain/result.h"

#include <cstdlib>

namespace {

// Translates a Result<void> into an HTTP response. This is structural
// plumbing, not business logic: no real payload handling happens here.
void RespondFromResult(const Result<void>& result, httplib::Response& res, int ok_status) {
    if (result.IsOk()) {
        res.status = ok_status;
        return;
    }
    const Error& error = result.GetError();
    if (error.code == ErrorCode::NotImplemented) {
        res.status = 501;
        res.set_content(error.message, "text/plain");
        return;
    }
    res.status = 500;
    res.set_content(error.message, "text/plain");
}

} // namespace

CameraApiController::CameraApiController(RegisterCameraUseCase& register_camera,
                                         ListCamerasUseCase& list_cameras,
                                         StartIngestUseCase& start_ingest,
                                         StopIngestUseCase& stop_ingest)
    : register_camera_(register_camera), list_cameras_(list_cameras), start_ingest_(start_ingest),
      stop_ingest_(stop_ingest) {}

void CameraApiController::RegisterRoutes(httplib::Server& server) {
    server.Post("/cameras", [this](const httplib::Request& req, httplib::Response& res) {
        // No JSON parsing library is available; real payload handling is
        // business logic and out of scope for this skeleton.
        (void)req;
        Camera camera{};
        Result<void> result = register_camera_.Execute(camera);
        RespondFromResult(result, res, 201);
    });

    server.Get("/cameras", [this](const httplib::Request& req, httplib::Response& res) {
        (void)req;
        Result<std::vector<Camera>> result = list_cameras_.Execute();
        if (result.IsOk()) {
            res.status = 200;
            return;
        }
        const Error& error = result.GetError();
        if (error.code == ErrorCode::NotImplemented) {
            res.status = 501;
            res.set_content(error.message, "text/plain");
            return;
        }
        res.status = 500;
        res.set_content(error.message, "text/plain");
    });

    server.Post(
        R"(/cameras/(\d+)/start)", [this](const httplib::Request& req, httplib::Response& res) {
            CameraId camera_id =
                static_cast<CameraId>(std::strtoull(req.matches[1].str().c_str(), nullptr, 10));
            Result<void> result = start_ingest_.Execute(camera_id);
            RespondFromResult(result, res, 200);
        });

    server.Post(
        R"(/cameras/(\d+)/stop)", [this](const httplib::Request& req, httplib::Response& res) {
            CameraId camera_id =
                static_cast<CameraId>(std::strtoull(req.matches[1].str().c_str(), nullptr, 10));
            Result<void> result = stop_ingest_.Execute(camera_id);
            RespondFromResult(result, res, 200);
        });
}
