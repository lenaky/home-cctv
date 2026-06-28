#pragma once
#include "domain/ports/outbound/IStreamEventPublisher.hpp"
#include <spdlog/spdlog.h>

namespace homecctv::adapters {

class LogEventPublisher : public domain::ports::IStreamEventPublisher {
public:
    void onStreamStarted(const std::string& camera_id) override {
        spdlog::info("[event] stream started: {}", camera_id);
    }
    void onStreamStopped(const std::string& camera_id) override {
        spdlog::info("[event] stream stopped: {}", camera_id);
    }
    void onStreamError(const std::string& camera_id, const std::string& error) override {
        spdlog::error("[event] stream error [{}]: {}", camera_id, error);
    }
    void onStreamReconnecting(const std::string& camera_id) override {
        spdlog::warn("[event] stream reconnecting: {}", camera_id);
    }
};

}  // namespace homecctv::adapters
