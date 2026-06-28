find_package(spdlog REQUIRED)
find_package(httplib REQUIRED)

add_executable(egress-server
    src/main.cpp
)

target_include_directories(egress-server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(egress-server PRIVATE
    spdlog::spdlog
    httplib::httplib
)

target_compile_options(egress-server PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wno-unused-parameter>
)
