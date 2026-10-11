add_library(egress_core
    domain/result.h
    domain/types.h
    domain/recording_metadata.h

    domain/ports/hls_segment_reader.h
    domain/ports/webrtc_egress_pipeline.h
    domain/ports/recording_storage_reader.h
    domain/ports/recording_query_repository.h

    application/get_live_manifest_usecase.h
    application/get_live_manifest_usecase.cpp
    application/create_webrtc_session_usecase.h
    application/create_webrtc_session_usecase.cpp
    application/list_recordings_usecase.h
    application/list_recordings_usecase.cpp
    application/get_recording_playback_usecase.h
    application/get_recording_playback_usecase.cpp

    adapters/inbound/http/live_api_controller.h
    adapters/inbound/http/live_api_controller.cpp
    adapters/inbound/http/playback_api_controller.h
    adapters/inbound/http/playback_api_controller.cpp

    adapters/outbound/hls/fs_hls_segment_reader.h
    adapters/outbound/hls/fs_hls_segment_reader.cpp

    adapters/outbound/webrtc/ffmpeg_webrtc_sender.h
    adapters/outbound/webrtc/ffmpeg_webrtc_sender.cpp

    adapters/outbound/storage/local_storage_reader.h
    adapters/outbound/storage/local_storage_reader.cpp
    adapters/outbound/storage/remote_storage_reader.h
    adapters/outbound/storage/remote_storage_reader.cpp

    adapters/outbound/sqlite/sqlite_recording_query_repository.h
    adapters/outbound/sqlite/sqlite_recording_query_repository.cpp
)

target_include_directories(egress_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(egress_core PUBLIC cxx_std_20)

# httplib::httplib is the CMake target exported by vcpkg's (and Homebrew's)
# cpp-httplib package.
find_package(httplib CONFIG REQUIRED)

# FFmpeg does not reliably export a CMake config package across vcpkg/Homebrew;
# consume it via PkgConfig instead (matches services/ingestion-server/CMakeLists.txt).
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavformat libavcodec libavutil)

# sqlite3 likewise has no consistent CMake config name across vcpkg
# (unofficial-sqlite3) and Homebrew (none) — consume it via PkgConfig.
pkg_check_modules(SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)

target_link_libraries(egress_core PUBLIC
    httplib::httplib
    PkgConfig::FFMPEG
    PkgConfig::SQLITE3
)

add_executable(egress-server main.cpp)
target_link_libraries(egress-server PRIVATE egress_core)
target_compile_features(egress-server PRIVATE cxx_std_20)

add_subdirectory(tests)
