find_package(GTest CONFIG REQUIRED)

add_executable(egress_tests
    domain/result_test.cpp
    application/get_live_manifest_usecase_test.cpp
    application/create_webrtc_session_usecase_test.cpp
    application/list_recordings_usecase_test.cpp
    application/get_recording_playback_usecase_test.cpp
    adapters/fs_hls_segment_reader_test.cpp
    adapters/ffmpeg_webrtc_sender_test.cpp
    adapters/local_storage_reader_test.cpp
    adapters/remote_storage_reader_test.cpp
    adapters/sqlite_recording_query_repository_test.cpp
    adapters/live_api_controller_test.cpp
    adapters/playback_api_controller_test.cpp
)

target_link_libraries(egress_tests PRIVATE
    egress_core
    GTest::gtest
    GTest::gtest_main
)

target_compile_features(egress_tests PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(egress_tests)
