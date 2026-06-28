#include "StreamManager.hpp"

#include <shared_mutex>

#include <spdlog/spdlog.h>

#include "adapters/inbound/rtsp/RtspReceiver.hpp"
#include "adapters/outbound/hls/LlHlsWriter.hpp"
#include "adapters/outbound/storage/FMp4Writer.hpp"

namespace homecctv::application {

StreamManager::StreamManager(std::shared_ptr<domain::ports::ICameraRepository> camera_repo,
                             std::shared_ptr<domain::ports::IStreamEventPublisher> event_publisher,
                             StreamManagerConfig config)
    : camera_repo_(std::move(camera_repo)),
      event_publisher_(std::move(event_publisher)),
      config_(std::move(config)) {}

StreamManager::~StreamManager() { stopAll(); }

Result<domain::StreamInfo> StreamManager::startStream(const std::string& camera_id) {
    {
        std::shared_lock rl(streams_mutex_);
        if (streams_.count(camera_id))
            return Result<domain::StreamInfo>::Err("Stream already active: " + camera_id);
    }

    auto cam_result = camera_repo_->findById(camera_id);
    if (cam_result.is_err()) return Result<domain::StreamInfo>::Err(cam_result.error());
    auto cam = cam_result.value();

    adapters::RtspOptions rtsp_opts;
    rtsp_opts.transport = (cam.settings.transport == domain::RtspTransport::Udp) ? "udp" : "tcp";

    adapters::LlHlsConfig ll_cfg;
    ll_cfg.segment_duration = cam.settings.segment_duration_sec;
    ll_cfg.list_size        = cam.settings.hls_list_size;

    auto stream = std::make_unique<ActiveStream>();

    stream->hls_writer = std::make_unique<adapters::LlHlsWriter>(
        config_.hls_root, camera_id, config_.egress_base_url, ll_cfg);

    if (cam.settings.recording_enabled) {
        auto rec_root = cam.settings.recording_path.empty()
                            ? config_.recording_root
                            : std::filesystem::path(cam.settings.recording_path);
        stream->recording_writer = std::make_unique<adapters::FMp4Writer>(rec_root, camera_id);
    }

    stream->info.camera_id = camera_id;
    stream->info.status = domain::StreamStatus::Connecting;
    stream->info.hls_playlist_url = stream->hls_writer->getPlaylistUrl();
    stream->info.started_at = std::chrono::system_clock::now();

    auto* hls_ptr = stream->hls_writer.get();
    auto* rec_ptr = stream->recording_writer.get();
    auto* flag_ptr = &stream->first_packet;
    auto* info_ptr = &stream->info;

    stream->receiver = std::make_unique<adapters::RtspReceiver>(cam.rtsp_url, rtsp_opts);

    stream->receiver->start(
        [this, camera_id, hls_ptr, rec_ptr, flag_ptr, info_ptr](AVPacket* pkt, const AVStream* src_st) {
            bool expected = true;
            if (flag_ptr->compare_exchange_strong(expected, false)) {
                hls_ptr->open(src_st);
                if (rec_ptr) rec_ptr->open(src_st);

                std::unique_lock wl(streams_mutex_);
                info_ptr->status = domain::StreamStatus::Streaming;
                info_ptr->codec_name = avcodec_get_name(src_st->codecpar->codec_id);
                info_ptr->width = src_st->codecpar->width;
                info_ptr->height = src_st->codecpar->height;
                wl.unlock();

                camera_repo_->updateStatus(camera_id, domain::CameraStatus::Active);
                event_publisher_->onStreamStarted(camera_id);
            }

            auto hls_res = hls_ptr->writePacket(pkt, src_st->time_base);
            if (hls_res.is_err())
                spdlog::error("HLS write error [{}]: {}", camera_id, hls_res.error());
            if (rec_ptr) rec_ptr->writePacket(pkt, src_st->time_base);
        },
        [this, camera_id](const std::string& error) {
            spdlog::warn("Stream error [{}]: {}", camera_id, error);
            camera_repo_->updateStatus(camera_id, domain::CameraStatus::Error, error);
            event_publisher_->onStreamError(camera_id, error);
        });

    domain::StreamInfo result_info = stream->info;

    std::unique_lock wl(streams_mutex_);
    streams_.emplace(camera_id, std::move(stream));

    return Result<domain::StreamInfo>::Ok(result_info);
}

Result<void> StreamManager::stopStream(const std::string& camera_id) {
    std::unique_ptr<ActiveStream> stream;
    {
        std::unique_lock wl(streams_mutex_);
        auto it = streams_.find(camera_id);
        if (it == streams_.end())
            return Result<void>::Err("No active stream: " + camera_id);
        stream = std::move(it->second);
        streams_.erase(it);
    }

    // Stop receiver outside the lock to avoid deadlock with packet callbacks
    stream->receiver->stop();
    stream->hls_writer->close();
    if (stream->recording_writer) stream->recording_writer->close();

    camera_repo_->updateStatus(camera_id, domain::CameraStatus::Inactive);
    event_publisher_->onStreamStopped(camera_id);
    spdlog::info("Stream stopped: {}", camera_id);
    return Result<void>::Ok();
}

Result<domain::StreamInfo> StreamManager::getStreamStatus(const std::string& camera_id) {
    std::shared_lock rl(streams_mutex_);
    auto it = streams_.find(camera_id);
    if (it == streams_.end()) {
        domain::StreamInfo idle;
        idle.camera_id = camera_id;
        idle.status = domain::StreamStatus::Idle;
        return Result<domain::StreamInfo>::Ok(idle);
    }
    return Result<domain::StreamInfo>::Ok(it->second->info);
}

void StreamManager::stopAll() {
    std::vector<std::string> ids;
    {
        std::shared_lock rl(streams_mutex_);
        for (auto& [id, _] : streams_) ids.push_back(id);
    }
    for (auto& id : ids) stopStream(id);
}

}  // namespace homecctv::application
