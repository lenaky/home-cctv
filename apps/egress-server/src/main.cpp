#include <csignal>
#include <filesystem>
#include <memory>

#include <httplib.h>
#include <spdlog/spdlog.h>

namespace {
std::function<void(int)> shutdown_handler;
}

static std::filesystem::path dataDir() {
#ifdef _WIN32
    auto* p = std::getenv("APPDATA");
    return std::filesystem::path(p ? p : ".") / "home-cctv";
#else
    auto* p = std::getenv("HOME");
    return std::filesystem::path(p ? p : ".") / ".home-cctv";
#endif
}

int main() {
    spdlog::set_level(spdlog::level::info);
    spdlog::info("home-cctv egress-server starting");

    int port = 8090;
    if (auto* p = std::getenv("EGRESS_PORT")) port = std::stoi(p);

    auto hls_root = dataDir() / "hls";
    std::filesystem::create_directories(hls_root);

    // Static frontend dist (optional — set FRONTEND_DIST env to enable)
    std::string frontend_dist;
    if (auto* p = std::getenv("FRONTEND_DIST")) frontend_dist = p;

    httplib::Server svr;

    // CORS for HLS (browsers require this for cross-origin HLS fetches)
    svr.set_pre_routing_handler([](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Range");
        res.set_header("Access-Control-Expose-Headers", "Content-Length, Content-Range");
        return httplib::Server::HandlerResponse::Unhandled;
    });

    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok","service":"egress"})", "application/json");
    });

    // Serve HLS manifests and segments
    svr.set_mount_point("/hls", hls_root.string());

    // Serve React frontend (production build)
    if (!frontend_dist.empty() && std::filesystem::exists(frontend_dist)) {
        svr.set_mount_point("/", frontend_dist);
        // SPA fallback — route unknown paths to index.html
        svr.Get(".*", [&frontend_dist](const httplib::Request&, httplib::Response& res) {
            auto index = std::filesystem::path(frontend_dist) / "index.html";
            if (std::filesystem::exists(index)) {
                std::ifstream f(index);
                res.set_content(std::string(std::istreambuf_iterator<char>(f), {}), "text/html");
            } else {
                res.status = 404;
            }
        });
    }

    shutdown_handler = [&](int) {
        spdlog::info("Egress shutting down...");
        svr.stop();
    };
    std::signal(SIGINT, [](int s) { shutdown_handler(s); });
    std::signal(SIGTERM, [](int s) { shutdown_handler(s); });

    spdlog::info("Egress server listening on 0.0.0.0:{}", port);
    svr.listen("0.0.0.0", port);

    spdlog::info("egress-server stopped");
    return 0;
}
