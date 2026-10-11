#pragma once
#include "domain/ports/live_packet_feed.h"
#include <memory>

std::unique_ptr<ILivePacketFeed> CreateLivePacketFeed(CameraId camera_id);
