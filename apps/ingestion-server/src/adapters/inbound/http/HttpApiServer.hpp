#pragma once
#include <memory>
#include <string>

#include <httplib.h>

#include "domain/ports/inbound/ICameraService.hpp"
#include "domain/ports/inbound/IStreamService.hpp"

namespace homecctv::adapters {

struct HttpApiConfig {
    std::string host = "0.0.0.0";
    int port = 8080;
    std::string hls_root;
    std::string cors_origin = "*";
};

class HttpApiServer {
public:
    HttpApiServer(std::shared_ptr<domain::ports::ICameraService> camera_svc,
                  std::shared_ptr<domain::ports::IStreamService> stream_svc,
                  HttpApiConfig config);
    ~HttpApiServer();

    void start();
    void stop();

private:
    void setupRoutes();
    void addCorsHeaders(httplib::Response& res);

    std::shared_ptr<domain::ports::ICameraService> camera_svc_;
    std::shared_ptr<domain::ports::IStreamService> stream_svc_;
    HttpApiConfig config_;
    std::unique_ptr<httplib::Server> server_;
};

}  // namespace homecctv::adapters
