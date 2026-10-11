cmake_minimum_required(VERSION 3.25)
project(home-cctv CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_subdirectory(services/ingestion-server)
add_subdirectory(services/egress-server)
