find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavformat libavcodec libavutil libswscale libswresample)

find_package(SQLite3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(httplib REQUIRED)

file(GLOB_RECURSE INGESTION_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(ingestion-server ${INGESTION_SOURCES})

target_include_directories(ingestion-server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ingestion-server PRIVATE
    homecctv::proto
    PkgConfig::FFMPEG
    SQLite3::SQLite3
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    httplib::httplib
)

target_compile_options(ingestion-server PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wno-unused-parameter>
)
