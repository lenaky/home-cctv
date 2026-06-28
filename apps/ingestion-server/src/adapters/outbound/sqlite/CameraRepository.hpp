#pragma once
#include <filesystem>
#include <memory>
#include <mutex>

#include <sqlite3.h>

#include "domain/ports/outbound/ICameraRepository.hpp"

namespace homecctv::adapters {

class CameraRepository : public domain::ports::ICameraRepository {
public:
    explicit CameraRepository(const std::filesystem::path& db_path);
    ~CameraRepository();

    Result<domain::Camera> findById(const std::string& id) override;
    Result<std::vector<domain::Camera>> findAll() override;
    Result<domain::Camera> save(const domain::Camera& camera) override;
    Result<domain::Camera> update(const domain::Camera& camera) override;
    Result<void> remove(const std::string& id) override;
    Result<void> updateStatus(const std::string& id, domain::CameraStatus status,
                               const std::optional<std::string>& error) override;

private:
    void initSchema();
    domain::Camera rowToCamera(sqlite3_stmt* stmt);

    sqlite3* db_ = nullptr;
    mutable std::mutex db_mutex_;
};

}  // namespace homecctv::adapters
