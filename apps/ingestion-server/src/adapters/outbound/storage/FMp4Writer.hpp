#pragma once
#include <filesystem>
#include <string>

#include "core/Result.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace homecctv::adapters {

struct FMp4Options {
    int segment_duration_sec = 3600;  // 1 hour per file
};

// Remuxes compatible streams to fMP4. Falls back to raw stream copy otherwise.
class FMp4Writer {
public:
    FMp4Writer(std::filesystem::path recording_dir, std::string camera_id,
               FMp4Options opts = {});
    ~FMp4Writer();

    Result<void> open(const AVStream* source_stream);
    Result<void> writePacket(const AVPacket* pkt, const AVRational& src_time_base);
    Result<void> rotate();
    Result<void> close();

    static bool isCompatibleCodec(AVCodecID codec_id);

private:
    Result<void> openSegment();

    std::filesystem::path recording_dir_;
    std::string camera_id_;
    FMp4Options opts_;
    AVFormatContext* ctx_ = nullptr;
    AVStream* out_stream_ = nullptr;
    const AVStream* src_stream_ = nullptr;
    bool use_fmp4_ = false;
    int64_t segment_start_pts_ = AV_NOPTS_VALUE;
    int segment_index_ = 0;
};

}  // namespace homecctv::adapters
