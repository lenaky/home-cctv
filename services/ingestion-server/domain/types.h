#pragma once
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

using CameraId = std::uint64_t;
using Timestamp = std::chrono::system_clock::time_point;

enum class StorageKind { Local, Remote };

struct StorageLocation {
    StorageKind kind;
    std::string path_or_uri;
};

struct EncodedPacket {
    std::vector<std::byte> data;
    Timestamp pts;
    bool is_key_frame;
};
