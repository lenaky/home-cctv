#pragma once
#include "domain/result.h"
#include "domain/types.h"

class ILivePacketFeed {
public:
    virtual ~ILivePacketFeed() = default;
    virtual Result<void> PublishPacket(CameraId camera_id, const EncodedPacket& packet) = 0;
};
