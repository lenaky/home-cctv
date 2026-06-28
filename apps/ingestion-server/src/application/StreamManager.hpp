#pragma once
#include <filesystem>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "domain/entities/Stream.hpp"
#include "domain/ports/inbound/IStreamService.hpp"
#include "domain/ports/outbound/ICameraRepository.hpp"
#include "domain/ports/outbound/IStreamEventPublisher.hpp"

namespace homecctv::adapters { class RtspReceiver; class LlHlsWriter; class FMp4Writer; }

namespace homecctv::application {

struct StreamManagerConfig {
    std::filesystem::path hls_root;
    std::filesystem::path recording_root;
    std::string egress_base_url;
};

class StreamManager : public domain::ports::IStreamService {
public:
    StreamManager(std::shared_ptr<domain::ports::ICameraRepository> camera_repo,
                  std::shared_ptr<domain::ports::IStreamEventPublisher> event_publisher,
                  StreamManagerConfig config);
    ~StreamManager();

    Result<domain::StreamInfo> startStream(const std::string& camera_id) override;
    Result<void> stopStream(const std::string& camera_id) override;
    Result<domain::StreamInfo> getStreamStatus(const std::string& camera_id) override;

    void stopAll();

private:
    struct ActiveStream {
        std::unique_ptr<adapters::RtspReceiver> receiver;
        std::unique_ptr<adapters::LlHlsWriter> hls_writer;
        std::unique_ptr<adapters::FMp4Writer> recording_writer;
        domain::StreamInfo info;
    };

    std::shared_ptr<domain::ports::ICameraRepository> camera_repo_;
    std::shared_ptr<domain::ports::IStreamEventPublisher> event_publisher_;
    StreamManagerConfig config_;

    std::unordered_map<std::string, std::unique_ptr<ActiveStream>> streams_;
    std::shared_mutex streams_mutex_;
};

}  // namespace homecctv::application
