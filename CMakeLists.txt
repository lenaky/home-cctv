cmake_minimum_required(VERSION 3.25)
project(home-cctv VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# vcpkg toolchain (optional — brew packages work without it)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain")
endif()

# Hint cmake to find brew-installed packages on macOS
if(APPLE)
    list(PREPEND CMAKE_PREFIX_PATH /opt/homebrew)
endif()

option(HOMECCTV_BUILD_TESTS "Build tests" OFF)

# packages/core requires gRPC — skip if not available (Phase 1 doesn't use gRPC at runtime)
find_package(gRPC CONFIG QUIET)
find_package(Protobuf CONFIG QUIET)
if(gRPC_FOUND AND Protobuf_FOUND)
    add_subdirectory(packages/core)
else()
    message(STATUS "gRPC/Protobuf not found — skipping packages/core (Phase 2 feature)")
    # Expose core headers directly so ingestion-server can include core/Result.hpp
    add_library(homecctv-proto INTERFACE)
    add_library(homecctv::proto ALIAS homecctv-proto)
    target_include_directories(homecctv-proto INTERFACE
        ${CMAKE_SOURCE_DIR}/packages/core/include)
endif()

add_subdirectory(apps/ingestion-server)
add_subdirectory(apps/egress-server)
