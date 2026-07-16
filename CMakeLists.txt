cmake_minimum_required(VERSION 3.10)
project(NetOrc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# Include directories
include_directories(
    src
    src/api/SouthBoundInterface/Classes
    src/api/SouthBoundInterface/Interfaces
    src/core/Classes
)

# Source files
set(SOURCES
    src/index.cpp
    src/core/Classes/App.cpp
    src/core/Classes/Observability.cpp
    src/api/SouthBoundInterface/Classes/SouthBoundInterface.cpp
    src/api/SouthBoundInterface/OpenFlowLib.cpp
    src/api/SouthBoundInterface/Utils/HandleData.cpp
    src/api/NorthBoundInterface/NorthBoundInterface.cpp
)

# Executable
add_executable(NetOrc ${SOURCES})

# Thread support and SQLite3
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# gRPC and Protobuf support
find_package(gRPC CONFIG QUIET)
if (gRPC_FOUND)
    set(GRPC_LINK_LIBS gRPC::grpc++)
else()
    pkg_check_modules(GRPCPP REQUIRED grpc++)
    link_directories(${GRPCPP_LIBRARY_DIRS})
    set(GRPC_LINK_LIBS ${GRPCPP_LIBRARIES})
endif()
find_package(Protobuf REQUIRED)

# Find the protoc and grpc_cpp_plugin executables
find_program(PROTOBUF_PROTOC_EXECUTABLE protoc)
find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

# Generate C++ code from .proto
set(PROTO_FILE src/api/MLInterface/proto/ml_service.proto)
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

set(PROTO_SRCS "${PROTO_GEN_DIR}/ml_service.pb.cc")
set(PROTO_HDRS "${PROTO_GEN_DIR}/ml_service.pb.h")
set(GRPC_SRCS "${PROTO_GEN_DIR}/ml_service.grpc.pb.cc")
set(GRPC_HDRS "${PROTO_GEN_DIR}/ml_service.grpc.pb.h")

add_custom_command(
    OUTPUT "${PROTO_SRCS}" "${PROTO_HDRS}" "${GRPC_SRCS}" "${GRPC_HDRS}"
    COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
    ARGS --cpp_out ${PROTO_GEN_DIR}
         --grpc_out ${PROTO_GEN_DIR}
         --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
         -I ${CMAKE_CURRENT_SOURCE_DIR}/src/api/MLInterface/proto
         ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
)

# Use include_directories for the generated headers
include_directories(
    ${SQLITE3_INCLUDE_DIRS} 
    ${PROTO_GEN_DIR} 
    ${Protobuf_INCLUDE_DIRS}
    ${GRPCPP_INCLUDE_DIRS}
    src/api/MLInterface
)

target_sources(NetOrc PRIVATE "${PROTO_SRCS}" "${GRPC_SRCS}")

target_link_libraries(NetOrc PRIVATE 
    Threads::Threads 
    ${SQLITE3_LIBRARIES}
    ${GRPC_LINK_LIBS}
    ${Protobuf_LIBRARIES}
)

# Validator unit tests
add_executable(validator_tests
    tests/validator_tests.cpp
)

target_include_directories(validator_tests PRIVATE
    src
    src/api/SouthBoundInterface/Classes
    src/api/SouthBoundInterface/Interfaces
    src/core/Classes
)

add_test(NAME validator_tests COMMAND $<TARGET_FILE:validator_tests>)

add_executable(app_integration_tests
    tests/app_integration_tests.cpp
    src/core/Classes/App.cpp
    src/core/Classes/Observability.cpp
    src/api/SouthBoundInterface/Classes/SouthBoundInterface.cpp
    src/api/SouthBoundInterface/OpenFlowLib.cpp
    src/api/NorthBoundInterface/NorthBoundInterface.cpp
)

target_include_directories(app_integration_tests PRIVATE
    src
    src/api/SouthBoundInterface/Classes
    src/api/SouthBoundInterface/Interfaces
    src/core/Classes
)

target_link_libraries(app_integration_tests PRIVATE Threads::Threads ${SQLITE3_LIBRARIES})
add_test(NAME app_integration_tests COMMAND $<TARGET_FILE:app_integration_tests>)
