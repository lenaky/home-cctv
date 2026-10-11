// Windows implementation: ILivePacketFeed over a Windows Named Pipe.
// Compiled only on WIN32 (selected by services/ingestion-server/CMakeLists.txt).
#include "adapters/outbound/ipc/live_packet_feed_factory.h"

#include <Windows.h>

#include <string>

namespace {

std::string PipeNameFor(CameraId camera_id) {
    return R"(\\.\pipe\home-cctv-ingest-)" + std::to_string(camera_id);
}

class NamedPipePacketFeed : public ILivePacketFeed {
public:
    explicit NamedPipePacketFeed(CameraId camera_id) : pipe_name_(PipeNameFor(camera_id)) {}

    Result<void> PublishPacket(CameraId camera_id, const EncodedPacket& packet) override {
        (void)camera_id;
        (void)packet;
        return Result<void>::Err(
            Error{ErrorCode::NotImplemented, "NamedPipePacketFeed::PublishPacket not implemented"});
    }

private:
    std::string pipe_name_;
};

} // namespace

std::unique_ptr<ILivePacketFeed> CreateLivePacketFeed(CameraId camera_id) {
    return std::make_unique<NamedPipePacketFeed>(camera_id);
}
