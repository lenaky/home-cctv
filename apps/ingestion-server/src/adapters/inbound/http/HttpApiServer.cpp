#include "HttpApiServer.hpp"

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <httplib.h>

#include "domain/entities/Camera.hpp"
#include "domain/entities/Stream.hpp"

using json = nlohmann::json;

namespace homecctv::adapters {

// ── JSON serialization helpers ────────────────────────────────────────────────

static json cameraToJson(const domain::Camera& cam) {
    return {
        {"id", cam.id},
        {"name", cam.name},
        {"rtsp_url", cam.rtsp_url},
        {"status", static_cast<int>(cam.status)},
        {"settings", {
            {"transport", static_cast<int>(cam.settings.transport)},
            {"segment_duration_sec", cam.settings.segment_duration_sec},
            {"hls_list_size", cam.settings.hls_list_size},
            {"recording_enabled", cam.settings.recording_enabled},
            {"recording_path", cam.settings.recording_path},
        }},
        {"created_at", std::chrono::duration_cast<std::chrono::seconds>(
                           cam.created_at.time_since_epoch()).count()},
        {"updated_at", std::chrono::duration_cast<std::chrono::seconds>(
                           cam.updated_at.time_since_epoch()).count()},
        {"last_error", cam.last_error.value_or("")},
    };
}

static json streamInfoToJson(const domain::StreamInfo& s) {
    return {
        {"camera_id", s.camera_id},
        {"status", static_cast<int>(s.status)},
        {"codec_name", s.codec_name},
        {"width", s.width},
        {"height", s.height},
        {"fps", s.fps},
        {"hls_playlist_url", s.hls_playlist_url},
        {"last_error", s.last_error.value_or("")},
    };
}

static domain::CameraSettings settingsFromJson(const json& j) {
    domain::CameraSettings s;
    if (j.contains("transport")) s.transport = static_cast<domain::RtspTransport>(j["transport"].get<int>());
    if (j.contains("segment_duration_sec")) s.segment_duration_sec = j["segment_duration_sec"];
    if (j.contains("hls_list_size")) s.hls_list_size = j["hls_list_size"];
    if (j.contains("recording_enabled")) s.recording_enabled = j["recording_enabled"];
    if (j.contains("recording_path")) s.recording_path = j["recording_path"];
    return s;
}

// ── Server ────────────────────────────────────────────────────────────────────

HttpApiServer::HttpApiServer(std::shared_ptr<domain::ports::ICameraService> camera_svc,
                             std::shared_ptr<domain::ports::IStreamService> stream_svc,
                             HttpApiConfig config)
    : camera_svc_(std::move(camera_svc)),
      stream_svc_(std::move(stream_svc)),
      config_(std::move(config)),
      server_(std::make_unique<httplib::Server>()) {
    setupRoutes();
}

HttpApiServer::~HttpApiServer() { stop(); }

void HttpApiServer::addCorsHeaders(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", config_.cors_origin);
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

void HttpApiServer::setupRoutes() {
    // CORS preflight
    server_->Options(".*", [this](const httplib::Request&, httplib::Response& res) {
        addCorsHeaders(res);
        res.status = 204;
    });

    // Health check
    server_->Get("/health", [this](const httplib::Request&, httplib::Response& res) {
        addCorsHeaders(res);
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    // GET /api/cameras
    server_->Get("/api/cameras", [this](const httplib::Request&, httplib::Response& res) {
        addCorsHeaders(res);
        auto result = camera_svc_->listCameras();
        if (result.is_err()) {
            res.status = 500;
            res.set_content(json{{"error", result.error()}}.dump(), "application/json");
            return;
        }
        json arr = json::array();
        for (auto& cam : result.value()) arr.push_back(cameraToJson(cam));
        res.set_content(arr.dump(), "application/json");
    });

    // GET /api/cameras/:id
    server_->Get("/api/cameras/:id", [this](const httplib::Request& req, httplib::Response& res) {
        addCorsHeaders(res);
        auto result = camera_svc_->getCamera(req.path_params.at("id"));
        if (result.is_err()) {
            res.status = 404;
            res.set_content(json{{"error", result.error()}}.dump(), "application/json");
            return;
        }
        res.set_content(cameraToJson(result.value()).dump(), "application/json");
    });

    // POST /api/cameras
    server_->Post("/api/cameras", [this](const httplib::Request& req, httplib::Response& res) {
        addCorsHeaders(res);
        try {
            auto body = json::parse(req.body);
            domain::ports::RegisterCameraCmd cmd;
            cmd.name = body.value("name", "");
            cmd.rtsp_url = body.value("rtsp_url", "");
            if (body.contains("settings")) cmd.settings = settingsFromJson(body["settings"]);

            auto result = camera_svc_->registerCamera(cmd);
            if (result.is_err()) {
                res.status = 400;
                res.set_content(json{{"error", result.error()}}.dump(), "application/json");
                return;
            }
            res.status = 201;
            res.set_content(cameraToJson(result.value()).dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    // PUT /api/cameras/:id
    server_->Put("/api/cameras/:id", [this](const httplib::Request& req, httplib::Response& res) {
        addCorsHeaders(res);
        try {
            auto body = json::parse(req.body);
            domain::ports::UpdateCameraCmd cmd;
            cmd.id = req.path_params.at("id");
            cmd.name = body.value("name", "");
            cmd.rtsp_url = body.value("rtsp_url", "");
            if (body.contains("settings")) cmd.settings = settingsFromJson(body["settings"]);

            auto result = camera_svc_->updateCamera(cmd);
            if (result.is_err()) {
                res.status = 404;
                res.set_content(json{{"error", result.error()}}.dump(), "application/json");
                return;
            }
            res.set_content(cameraToJson(result.value()).dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
        }
    });

    // DELETE /api/cameras/:id
    server_->Delete("/api/cameras/:id", [this](const httplib::Request& req, httplib::Response& res) {
        addCorsHeaders(res);
        auto result = camera_svc_->deleteCamera(req.path_params.at("id"));
        if (result.is_err()) {
            res.status = 404;
            res.set_content(json{{"error", result.error()}}.dump(), "application/json");
            return;
        }
        res.status = 204;
    });

    // POST /api/streams/:camera_id/start
    server_->Post("/api/streams/:camera_id/start",
                  [this](const httplib::Request& req, httplib::Response& res) {
        addCorsHeaders(res);
        auto result = stream_svc_->startStream(req.path_params.at("camera_id"));
        if (result.is_err()) {
            res.status = 400;
            res.set_content(json{{"error", result.error()}}.dump(), "application/json");
            return;
        }
        res.set_content(streamInfoToJson(result.value()).dump(), "application/json");
    });

    // POST /api/streams/:camera_id/stop
    server_->Post("/api/streams/:camera_id/stop",
                  [this](const httplib::Request& req, httplib::Response& res) {
        addCorsHeaders(res);
        auto result = stream_svc_->stopStream(req.path_params.at("camera_id"));
        if (result.is_err()) {
            res.status = 400;
            res.set_content(json{{"error", result.error()}}.dump(), "application/json");
            return;
        }
        res.status = 200;
        res.set_content(R"({"success":true})", "application/json");
    });

    // GET /api/streams/:camera_id/status
    server_->Get("/api/streams/:camera_id/status",
                 [this](const httplib::Request& req, httplib::Response& res) {
        addCorsHeaders(res);
        auto result = stream_svc_->getStreamStatus(req.path_params.at("camera_id"));
        if (result.is_err()) {
            res.status = 404;
            res.set_content(json{{"error", result.error()}}.dump(), "application/json");
            return;
        }
        res.set_content(streamInfoToJson(result.value()).dump(), "application/json");
    });

    // Serve HLS segments from ingestion server's hls_root
    if (!config_.hls_root.empty()) {
        server_->set_mount_point("/hls", config_.hls_root);
    }
}

void HttpApiServer::start() {
    spdlog::info("Ingestion HTTP API listening on {}:{}", config_.host, config_.port);
    server_->listen(config_.host, config_.port);
}

void HttpApiServer::stop() {
    server_->stop();
}

}  // namespace homecctv::adapters
