// macOS/APPLE implementation: ILivePacketFeed over a Unix Domain Socket.
// Compiled only on APPLE (selected by services/ingestion-server/CMakeLists.txt).
#include "adapters/outbound/ipc/live_packet_feed_factory.h"

#include <sys/socket.h>
#include <sys/un.h>

#include <string>

namespace {

std::string SocketPathFor(CameraId camera_id) {
    return "/tmp/home-cctv/ingest/" + std::to_string(camera_id) + ".sock";
}

class UdsPacketFeed : public ILivePacketFeed {
public:
    explicit UdsPacketFeed(CameraId camera_id) : socket_path_(SocketPathFor(camera_id)) {}

    Result<void> PublishPacket(CameraId camera_id, const EncodedPacket& packet) override {
        (void)camera_id;
        (void)packet;
        return Result<void>::Err(
            Error{ErrorCode::NotImplemented, "UdsPacketFeed::PublishPacket not implemented"});
    }

private:
    std::string socket_path_;
};

} // namespace

std::unique_ptr<ILivePacketFeed> CreateLivePacketFeed(CameraId camera_id) {
    return std::make_unique<UdsPacketFeed>(camera_id);
}
