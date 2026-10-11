set(INGESTION_CORE_SOURCES
    application/register_camera_usecase.cpp
    application/list_cameras_usecase.cpp
    application/start_ingest_usecase.cpp
    application/stop_ingest_usecase.cpp
    adapters/inbound/http/camera_api_controller.cpp
    adapters/outbound/sqlite/sqlite_camera_repository.cpp
    adapters/outbound/sqlite/sqlite_recording_repository.cpp
    adapters/outbound/pipeline/ffmpeg_ingest_pipeline.cpp
    adapters/outbound/storage/local_storage_writer.cpp
    adapters/outbound/storage/remote_storage_writer.cpp
)

if(APPLE)
    list(APPEND INGESTION_CORE_SOURCES adapters/outbound/ipc/uds_packet_feed.cpp)
elseif(WIN32)
    list(APPEND INGESTION_CORE_SOURCES adapters/outbound/ipc/named_pipe_packet_feed.cpp)
endif()

add_library(ingestion_core ${INGESTION_CORE_SOURCES})

target_include_directories(ingestion_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(ingestion_core PUBLIC cxx_std_20)

# httplib::httplib is the CMake target exported by vcpkg's cpp-httplib port.
find_package(httplib CONFIG REQUIRED)

# FFmpeg does not export a vcpkg CMake config package; the ffmpeg vcpkg port
# installs plain libraries/headers meant to be consumed via PkgConfig, or
# linked directly by name (avformat, avcodec, avutil). Using PkgConfig here;
# verify this resolves correctly once the real vcpkg install is tested.
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavformat libavcodec libavutil)

# sqlite3 has no consistent CMake config name across vcpkg (unofficial-sqlite3)
# and Homebrew (none) — consume it via PkgConfig instead (same approach as FFmpeg).
pkg_check_modules(SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)

target_link_libraries(ingestion_core
    PUBLIC
        httplib::httplib
        PkgConfig::FFMPEG
        PkgConfig::SQLITE3
)

add_executable(ingestion-server main.cpp)
target_link_libraries(ingestion-server PRIVATE ingestion_core)
target_compile_features(ingestion-server PRIVATE cxx_std_20)

add_subdirectory(tests)
