#include "CameraRepository.hpp"

#include <chrono>
#include <stdexcept>

#include <spdlog/spdlog.h>
#include <sqlite3.h>

namespace homecctv::adapters {

static int64_t toUnix(std::chrono::system_clock::time_point tp) {
    return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}

static std::chrono::system_clock::time_point fromUnix(int64_t ts) {
    return std::chrono::system_clock::time_point{std::chrono::seconds{ts}};
}

CameraRepository::CameraRepository(const std::filesystem::path& db_path) {
    std::filesystem::create_directories(db_path.parent_path());
    if (sqlite3_open(db_path.string().c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error("Failed to open SQLite database: " + db_path.string());
    }
    sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    initSchema();
}

CameraRepository::~CameraRepository() {
    if (db_) sqlite3_close(db_);
}

void CameraRepository::initSchema() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS cameras (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            rtsp_url TEXT NOT NULL,
            status INTEGER NOT NULL DEFAULT 1,
            transport INTEGER NOT NULL DEFAULT 0,
            segment_duration_sec INTEGER NOT NULL DEFAULT 2,
            hls_list_size INTEGER NOT NULL DEFAULT 10,
            recording_enabled INTEGER NOT NULL DEFAULT 0,
            recording_path TEXT NOT NULL DEFAULT '',
            created_at INTEGER NOT NULL,
            updated_at INTEGER NOT NULL,
            last_error TEXT
        );
    )";
    char* errmsg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        std::string err = errmsg;
        sqlite3_free(errmsg);
        throw std::runtime_error("Schema init failed: " + err);
    }
}

domain::Camera CameraRepository::rowToCamera(sqlite3_stmt* stmt) {
    domain::Camera cam;
    cam.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    cam.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    cam.rtsp_url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    cam.status = static_cast<domain::CameraStatus>(sqlite3_column_int(stmt, 3));
    cam.settings.transport = static_cast<domain::RtspTransport>(sqlite3_column_int(stmt, 4));
    cam.settings.segment_duration_sec = sqlite3_column_int(stmt, 5);
    cam.settings.hls_list_size = sqlite3_column_int(stmt, 6);
    cam.settings.recording_enabled = sqlite3_column_int(stmt, 7) != 0;
    if (auto* p = sqlite3_column_text(stmt, 8))
        cam.settings.recording_path = reinterpret_cast<const char*>(p);
    cam.created_at = fromUnix(sqlite3_column_int64(stmt, 9));
    cam.updated_at = fromUnix(sqlite3_column_int64(stmt, 10));
    if (auto* e = sqlite3_column_text(stmt, 11))
        cam.last_error = reinterpret_cast<const char*>(e);
    return cam;
}

Result<domain::Camera> CameraRepository::findById(const std::string& id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    const char* sql =
        "SELECT id,name,rtsp_url,status,transport,segment_duration_sec,hls_list_size,"
        "recording_enabled,recording_path,created_at,updated_at,last_error "
        "FROM cameras WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);

    Result<domain::Camera> result = Result<domain::Camera>::Err("Camera not found: " + id);
    if (sqlite3_step(stmt) == SQLITE_ROW) result = Result<domain::Camera>::Ok(rowToCamera(stmt));
    sqlite3_finalize(stmt);
    return result;
}

Result<std::vector<domain::Camera>> CameraRepository::findAll() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    const char* sql =
        "SELECT id,name,rtsp_url,status,transport,segment_duration_sec,hls_list_size,"
        "recording_enabled,recording_path,created_at,updated_at,last_error "
        "FROM cameras ORDER BY created_at ASC;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    std::vector<domain::Camera> cameras;
    while (sqlite3_step(stmt) == SQLITE_ROW) cameras.push_back(rowToCamera(stmt));
    sqlite3_finalize(stmt);
    return Result<std::vector<domain::Camera>>::Ok(std::move(cameras));
}

Result<domain::Camera> CameraRepository::save(const domain::Camera& cam) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    const char* sql =
        "INSERT INTO cameras(id,name,rtsp_url,status,transport,segment_duration_sec,"
        "hls_list_size,recording_enabled,recording_path,created_at,updated_at,last_error)"
        " VALUES(?,?,?,?,?,?,?,?,?,?,?,?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, cam.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, cam.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, cam.rtsp_url.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, static_cast<int>(cam.status));
    sqlite3_bind_int(stmt, 5, static_cast<int>(cam.settings.transport));
    sqlite3_bind_int(stmt, 6, cam.settings.segment_duration_sec);
    sqlite3_bind_int(stmt, 7, cam.settings.hls_list_size);
    sqlite3_bind_int(stmt, 8, cam.settings.recording_enabled ? 1 : 0);
    sqlite3_bind_text(stmt, 9, cam.settings.recording_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 10, toUnix(cam.created_at));
    sqlite3_bind_int64(stmt, 11, toUnix(cam.updated_at));
    if (cam.last_error) sqlite3_bind_text(stmt, 12, cam.last_error->c_str(), -1, SQLITE_STATIC);
    else sqlite3_bind_null(stmt, 12);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    if (!ok) return Result<domain::Camera>::Err("Failed to save camera");
    return Result<domain::Camera>::Ok(cam);
}

Result<domain::Camera> CameraRepository::update(const domain::Camera& cam) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    const char* sql =
        "UPDATE cameras SET name=?,rtsp_url=?,transport=?,segment_duration_sec=?,"
        "hls_list_size=?,recording_enabled=?,recording_path=?,updated_at=? WHERE id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, cam.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, cam.rtsp_url.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, static_cast<int>(cam.settings.transport));
    sqlite3_bind_int(stmt, 4, cam.settings.segment_duration_sec);
    sqlite3_bind_int(stmt, 5, cam.settings.hls_list_size);
    sqlite3_bind_int(stmt, 6, cam.settings.recording_enabled ? 1 : 0);
    sqlite3_bind_text(stmt, 7, cam.settings.recording_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 8, toUnix(cam.updated_at));
    sqlite3_bind_text(stmt, 9, cam.id.c_str(), -1, SQLITE_STATIC);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    if (!ok) return Result<domain::Camera>::Err("Failed to update camera");
    return Result<domain::Camera>::Ok(cam);
}

Result<void> CameraRepository::remove(const std::string& id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, "DELETE FROM cameras WHERE id=?;", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok ? Result<void>::Ok() : Result<void>::Err("Failed to delete camera");
}

Result<void> CameraRepository::updateStatus(const std::string& id, domain::CameraStatus status,
                                             const std::optional<std::string>& error) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    auto now = toUnix(std::chrono::system_clock::now());
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_,
                       "UPDATE cameras SET status=?,last_error=?,updated_at=? WHERE id=?;",
                       -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, static_cast<int>(status));
    if (error) sqlite3_bind_text(stmt, 2, error->c_str(), -1, SQLITE_STATIC);
    else sqlite3_bind_null(stmt, 2);
    sqlite3_bind_int64(stmt, 3, now);
    sqlite3_bind_text(stmt, 4, id.c_str(), -1, SQLITE_STATIC);
    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok ? Result<void>::Ok() : Result<void>::Err("Failed to update camera status");
}

}  // namespace homecctv::adapters
