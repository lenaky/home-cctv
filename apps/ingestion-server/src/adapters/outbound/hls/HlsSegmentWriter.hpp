#pragma once
#include <filesystem>
#include <string>

#include "domain/ports/outbound/ISegmentWriter.hpp"

extern "C" {
#include <libavformat/avformat.h>
}

namespace homecctv::adapters {

struct HlsOptions {
    int segment_duration = 2;
    int list_size = 10;
    bool low_latency = false;
};

class HlsSegmentWriter : public domain::ports::ISegmentWriter {
public:
    HlsSegmentWriter(std::filesystem::path hls_root, std::string camera_id,
                     std::string base_url, HlsOptions opts = {});
    ~HlsSegmentWriter() override;

    Result<void> open(const AVStream* source_stream) override;
    Result<void> writePacket(const AVPacket* pkt, const AVRational& src_time_base) override;
    Result<void> close() override;
    std::string getPlaylistUrl() const override;

private:
    std::filesystem::path hls_root_;
    std::string camera_id_;
    std::string base_url_;
    HlsOptions opts_;
    AVFormatContext* hls_ctx_ = nullptr;
    AVStream* out_stream_ = nullptr;
    bool errored_ = false;
    int64_t next_dts_ = 0;
};

}  // namespace homecctv::adapters
