#pragma once
#include <string>

#include "core/Result.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace homecctv::domain::ports {

class ISegmentWriter {
public:
    virtual ~ISegmentWriter() = default;
    virtual Result<void> open(const AVStream* source_stream) = 0;
    virtual Result<void> writePacket(const AVPacket* pkt, const AVRational& src_time_base) = 0;
    virtual Result<void> close() = 0;
    virtual std::string getPlaylistUrl() const = 0;
};

}  // namespace homecctv::domain::ports
