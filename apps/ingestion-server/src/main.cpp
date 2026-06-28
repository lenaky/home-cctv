#include <csignal>
#include <filesystem>
#include <memory>
#include <thread>

#include <spdlog/spdlog.h>

extern "C" {
#include <libavformat/avformat.h>
}

#include "adapters/inbound/http/HttpApiServer.hpp"
#include "adapters/outbound/events/LogEventPublisher.hpp"
#include "adapters/outbound/sqlite/CameraRepository.hpp"
#include "application/CameraService.hpp"
#include "application/StreamManager.hpp"

namespace {
std::function<void(int)> shutdown_handler;
}

static std::filesystem::path dataDir() {
#ifdef _WIN32
    auto* app_data = std::getenv("APPDATA");
    return std::filesystem::path(app_data ? app_data : ".") / "home-cctv";
#else
    auto* home = std::getenv("HOME");
    return std::filesystem::path(home ? home : ".") / ".home-cctv";
#endif
}

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::info);
    spdlog::info("home-cctv ingestion-server starting");

    // FFmpeg logging
    av_log_set_level(AV_LOG_WARNING);

    // ── Paths ──────────────────────────────────────────────────────────────────
    auto data = dataDir();
    auto db_path = data / "ingestion.db";
    auto hls_root = data / "hls";
    auto recording_root = data / "recordings";

    std::filesystem::create_directories(hls_root);
    std::filesystem::create_directories(recording_root);

    spdlog::info("Data dir: {}", data.string());

    // ── Config from env ────────────────────────────────────────────────────────
    int api_port = 8080;
    if (auto* p = std::getenv("INGESTION_PORT")) api_port = std::stoi(p);

    std::string egress_base_url = "http://localhost:8090";
    if (auto* p = std::getenv("EGRESS_BASE_URL")) egress_base_url = p;

    // ── Dependency wiring (hexagonal composition root) ─────────────────────────
    auto camera_repo = std::make_shared<homecctv::adapters::CameraRepository>(db_path);
    auto event_publisher = std::make_shared<homecctv::adapters::LogEventPublisher>();
    auto camera_svc = std::make_shared<homecctv::application::CameraService>(camera_repo);

    homecctv::application::StreamManagerConfig sm_config{
        .hls_root = hls_root,
        .recording_root = recording_root,
        .egress_base_url = egress_base_url,
    };
    auto stream_mgr = std::make_shared<homecctv::application::StreamManager>(
        camera_repo, event_publisher, sm_config);

    homecctv::adapters::HttpApiConfig api_config{
        .host = "0.0.0.0",
        .port = api_port,
        .hls_root = hls_root.string(),
    };
    auto http_server = std::make_unique<homecctv::adapters::HttpApiServer>(
        camera_svc, stream_mgr, api_config);

    // ── Graceful shutdown ──────────────────────────────────────────────────────
    shutdown_handler = [&](int) {
        spdlog::info("Shutting down...");
        stream_mgr->stopAll();
        http_server->stop();
    };
    std::signal(SIGINT, [](int s) { shutdown_handler(s); });
    std::signal(SIGTERM, [](int s) { shutdown_handler(s); });

    // ── Run (blocking) ─────────────────────────────────────────────────────────
    http_server->start();

    spdlog::info("ingestion-server stopped");
    return 0;
}
