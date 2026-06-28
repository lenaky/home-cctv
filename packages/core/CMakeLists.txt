find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(PROTO_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

set(PROTO_FILES
    ${PROTO_DIR}/camera_service.proto
    ${PROTO_DIR}/stream_service.proto
)

add_library(homecctv-proto STATIC)

protobuf_generate(
    TARGET homecctv-proto
    LANGUAGE cpp
    PROTOS ${PROTO_FILES}
    IMPORT_DIRS ${PROTO_DIR}
    PROTOC_OUT_DIR ${PROTO_OUT_DIR}
)

protobuf_generate(
    TARGET homecctv-proto
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    PROTOS ${PROTO_FILES}
    IMPORT_DIRS ${PROTO_DIR}
    PROTOC_OUT_DIR ${PROTO_OUT_DIR}
)

target_include_directories(homecctv-proto PUBLIC
    ${PROTO_OUT_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(homecctv-proto PUBLIC
    gRPC::grpc++
    protobuf::libprotobuf
)

add_library(homecctv::proto ALIAS homecctv-proto)
