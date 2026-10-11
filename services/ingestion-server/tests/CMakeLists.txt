find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(ingestion_tests
    domain/result_test.cpp
    application/register_camera_usecase_test.cpp
    application/list_cameras_usecase_test.cpp
    application/start_ingest_usecase_test.cpp
    application/stop_ingest_usecase_test.cpp
    adapters/sqlite_camera_repository_test.cpp
    adapters/sqlite_recording_repository_test.cpp
    adapters/ffmpeg_ingest_pipeline_test.cpp
    adapters/local_storage_writer_test.cpp
    adapters/remote_storage_writer_test.cpp
    adapters/camera_api_controller_test.cpp
)

target_include_directories(ingestion_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_compile_features(ingestion_tests PRIVATE cxx_std_20)

target_link_libraries(ingestion_tests
    PRIVATE
        ingestion_core
        GTest::gtest
        GTest::gtest_main
)

gtest_discover_tests(ingestion_tests)
