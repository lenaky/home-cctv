#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "domain/ports/outbound/ISegmentWriter.hpp"

extern "C" {
#include <libavformat/avformat.h>
}

namespace homecctv::adapters {

struct LlHlsConfig {
    double part_duration    = 0.2;  // target part duration in seconds
    double segment_duration = 2.0;  // target full segment duration in seconds
    int    list_size        = 5;    // number of complete segments to keep in playlist
};

class LlHlsWriter : public domain::ports::ISegmentWriter {
public:
    LlHlsWriter(std::filesystem::path hls_root, std::string camera_id,
                std::string base_url, LlHlsConfig cfg = {});
    ~LlHlsWriter() override;

    // Call setAudioStream BEFORE open() to include an audio track in init.mp4.
    void setAudioStream(const AVStream* audio_stream);

    Result<void> open(const AVStream* source_stream) override;
    Result<void> writePacket(const AVPacket* pkt, const AVRational& src_time_base) override;
    Result<void> writeAudioPacket(const AVPacket* pkt, const AVRational& src_time_base);
    Result<void> close() override;
    std::string  getPlaylistUrl() const override;

private:
    struct Part {
        double  duration;
        int64_t byte_offset;
        int64_t byte_length;
        bool    independent;
    };
    struct Segment {
        int             seq;
        std::string     filename;
        double          duration;
        std::vector<Part> parts;
    };

    void writeInitSegment(const AVStream* video_stream);
    void openSegment(int seq);
    void flushPart(bool independent);
    void finalizeSegment();
    void writePlaylist(bool end_of_stream = false);

    std::filesystem::path hls_root_;
    std::string           camera_id_;
    std::filesystem::path cam_dir_;
    std::string           base_url_;
    LlHlsConfig           cfg_;

    AVFormatContext*  ctx_              = nullptr;
    AVStream*         out_video_stream_ = nullptr;
    AVStream*         out_audio_stream_ = nullptr;
    const AVStream*   pending_audio_    = nullptr;  // set before open()

    // Video timestamp state
    int64_t    last_dts_  = AV_NOPTS_VALUE;
    int64_t    next_dts_  = 0;
    AVRational src_tb_    = {1, 90000};

    // Audio timestamp state
    int64_t    audio_last_dts_ = AV_NOPTS_VALUE;
    int64_t    audio_next_dts_ = 0;

    // Segment / part tracking
    int    current_seg_seq_      = 0;
    double current_seg_duration_ = 0.0;
    double current_part_duration_= 0.0;
    int64_t part_start_offset_   = 0;
    bool   next_part_independent_= true;
    bool   initialized_          = false;
    bool   errored_              = false;

    Segment              current_seg_;
    std::vector<Segment> completed_segs_;
};

}  // namespace homecctv::adapters
