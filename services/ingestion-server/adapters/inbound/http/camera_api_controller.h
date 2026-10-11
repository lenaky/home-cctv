#pragma once
#include "application/list_cameras_usecase.h"
#include "application/register_camera_usecase.h"
#include "application/start_ingest_usecase.h"
#include "application/stop_ingest_usecase.h"
#include <httplib.h>

class CameraApiController {
public:
    CameraApiController(RegisterCameraUseCase& register_camera, ListCamerasUseCase& list_cameras,
                        StartIngestUseCase& start_ingest, StopIngestUseCase& stop_ingest);

    void RegisterRoutes(httplib::Server& server);

private:
    RegisterCameraUseCase& register_camera_;
    ListCamerasUseCase& list_cameras_;
    StartIngestUseCase& start_ingest_;
    StopIngestUseCase& stop_ingest_;
};
