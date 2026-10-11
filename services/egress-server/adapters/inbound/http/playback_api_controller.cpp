#include "adapters/inbound/http/playback_api_controller.h"

#include <cstdlib>
#include <string>

PlaybackApiController::PlaybackApiController(ListRecordingsUseCase& list_recordings,
                                             GetRecordingPlaybackUseCase& get_playback)
    : list_recordings_(list_recordings), get_playback_(get_playback) {}

namespace {

void WriteErrorResponse(httplib::Response& res, const Error& error) {
    if (error.code == ErrorCode::NotImplemented) {
        res.status = 501;
    } else if (error.code == ErrorCode::NotFound) {
        res.status = 404;
    } else if (error.code == ErrorCode::InvalidArgument) {
        res.status = 400;
    } else {
        res.status = 500;
    }
    res.set_content(error.message, "text/plain");
}

Timestamp ParseTimestampParam(const httplib::Request& req, const char* name) {
    long long seconds = 0;
    if (req.has_param(name)) {
        seconds = std::strtoll(req.get_param_value(name).c_str(), nullptr, 10);
    }
    return Timestamp(std::chrono::seconds(seconds));
}

Timestamp ParseTimestampPathSegment(const std::string& segment) {
    long long seconds = std::strtoll(segment.c_str(), nullptr, 10);
    return Timestamp(std::chrono::seconds(seconds));
}

} // namespace

void PlaybackApiController::RegisterRoutes(httplib::Server& server) {
    server.Get(R"(/cameras/(\d+)/recordings)", [this](const httplib::Request& req,
                                                      httplib::Response& res) {
        CameraId camera_id =
            static_cast<CameraId>(std::strtoull(req.matches[1].str().c_str(), nullptr, 10));
        Timestamp from = ParseTimestampParam(req, "from");
        Timestamp to = ParseTimestampParam(req, "to");
        auto result = list_recordings_.Execute(camera_id, from, to);
        if (result.IsErr()) {
            WriteErrorResponse(res, result.GetError());
            return;
        }
        res.set_content("", "text/plain");
    });

    server.Get(R"(/cameras/(\d+)/recordings/(\d+)/(.+))", [this](const httplib::Request& req,
                                                                 httplib::Response& res) {
        CameraId camera_id =
            static_cast<CameraId>(std::strtoull(req.matches[1].str().c_str(), nullptr, 10));
        Timestamp at = ParseTimestampPathSegment(req.matches[2].str());
        std::string segment_name = req.matches[3].str();
        auto result = get_playback_.Execute(camera_id, at, segment_name);
        if (result.IsErr()) {
            WriteErrorResponse(res, result.GetError());
            return;
        }
        const auto& bytes = result.Value();
        res.set_content(reinterpret_cast<const char*>(bytes.data()), bytes.size(),
                        "application/octet-stream");
    });
}
