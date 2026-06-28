#include <csignal>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>

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

// Parse current LL-HLS playlist state.
// Returns {current_msn, current_part_count} where:
//   current_msn  = EXT-X-MEDIA-SEQUENCE + count of complete EXTINF segments
//   current_part = count of EXT-X-PART lines after the last EXTINF
static std::pair<int, int> parsePlaylistState(const std::string& content) {
    int media_seq    = -1;
    int extinf_count = 0;
    int current_parts= 0;

    std::istringstream ss(content);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.rfind("#EXT-X-MEDIA-SEQUENCE:", 0) == 0) {
            try { media_seq = std::stoi(line.substr(22)); } catch (...) {}
        } else if (line.rfind("#EXTINF:", 0) == 0) {
            extinf_count++;
            current_parts = 0;
        } else if (line.rfind("#EXT-X-PART:", 0) == 0) {
            current_parts++;
        }
    }

    if (media_seq < 0) return {-1, -1};
    return {media_seq + extinf_count, current_parts};
}

static bool playlistSatisfies(const std::string& content, int wanted_msn, int wanted_part) {
    auto [msn, parts] = parsePlaylistState(content);
    if (msn < 0) return false;
    if (msn > wanted_msn) return true;
    if (msn == wanted_msn) return parts >= wanted_part + 1;
    return false;
}

// Extract cam_id from /hls/<cam_id>/index.m3u8; returns "" if no match.
static std::string extractCamId(const std::string& path) {
    const std::string prefix = "/hls/";
    if (path.rfind(prefix, 0) != 0) return {};
    auto rest = path.substr(prefix.size());
    auto slash = rest.find('/');
    if (slash == std::string::npos) return {};
    if (rest.substr(slash + 1) != "index.m3u8") return {};
    return rest.substr(0, slash);
}

static void servePlaylistBlocking(const std::filesystem::path& hls_root,
                                   const std::string& cam_id,
                                   int wanted_msn, int wanted_part,
                                   httplib::Response& res) {
    auto playlist_path = hls_root / cam_id / "index.m3u8";
    const auto deadline      = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    const auto poll_interval = std::chrono::milliseconds(50);

    std::string content;
    while (true) {
        if (std::ifstream f(playlist_path); f.is_open())
            content.assign(std::istreambuf_iterator<char>(f), {});

        bool satisfied = (wanted_msn < 0) ||
                         (!content.empty() && playlistSatisfies(content, wanted_msn, wanted_part));

        if (satisfied && !content.empty()) break;
        if (std::chrono::steady_clock::now() >= deadline) break;
        std::this_thread::sleep_for(poll_interval);
    }

    if (content.empty()) { res.status = 404; return; }

    res.set_content(content, "application/vnd.apple.mpegurl");
    res.set_header("Cache-Control", "no-cache");
}

int main() {
    spdlog::set_level(spdlog::level::info);
    spdlog::info("home-cctv egress-server starting");

    int port = 8090;
    if (auto* p = std::getenv("EGRESS_PORT")) port = std::stoi(p);

    auto hls_root = dataDir() / "hls";
    std::filesystem::create_directories(hls_root);

    std::string frontend_dist;
    if (auto* p = std::getenv("FRONTEND_DIST")) frontend_dist = p;

    httplib::Server svr;

    // Pre-routing: runs before mount-point file serving.
    // Handles CORS + LL-HLS blocking playlist reload for index.m3u8 requests.
    svr.set_pre_routing_handler([&hls_root](const httplib::Request& req,
                                             httplib::Response& res) {
        // CORS headers on every response
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Range");
        res.set_header("Access-Control-Expose-Headers", "Content-Length, Content-Range");

        // Intercept index.m3u8 before the static mount point can serve it
        if (req.method != "GET") return httplib::Server::HandlerResponse::Unhandled;

        std::string cam_id = extractCamId(req.path);
        if (cam_id.empty()) return httplib::Server::HandlerResponse::Unhandled;

        int wanted_msn  = -1;
        int wanted_part = -1;
        if (req.has_param("_HLS_msn"))  { try { wanted_msn  = std::stoi(req.get_param_value("_HLS_msn"));  } catch (...) {} }
        if (req.has_param("_HLS_part")) { try { wanted_part = std::stoi(req.get_param_value("_HLS_part")); } catch (...) {} }

        servePlaylistBlocking(hls_root, cam_id, wanted_msn, wanted_part, res);
        return httplib::Server::HandlerResponse::Handled;
    });

    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok","service":"egress"})", "application/json");
    });

    // Static HLS file serving — segments + init.mp4 (byte-range support built into httplib)
    svr.set_mount_point("/hls", hls_root.string());

    // Serve React frontend (production build)
    if (!frontend_dist.empty() && std::filesystem::exists(frontend_dist)) {
        svr.set_mount_point("/", frontend_dist);
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
    std::signal(SIGINT,  [](int s) { shutdown_handler(s); });
    std::signal(SIGTERM, [](int s) { shutdown_handler(s); });

    spdlog::info("Egress server listening on 0.0.0.0:{}", port);
    svr.listen("0.0.0.0", port);

    spdlog::info("egress-server stopped");
    return 0;
}
