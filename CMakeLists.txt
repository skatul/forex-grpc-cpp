cmake_minimum_required(VERSION 3.10)
project(forex-grpc-cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find gRPC
find_package(gRPC REQUIRED)

# Find CURL
find_package(CURL REQUIRED)

# Include directories
include_directories(include include_external)

# Add source files
add_executable(forex_service src/forex_service.cpp include/forex.grpc.pb.cc include/forex.pb.cc)

# Link libraries
target_link_libraries(forex_service gRPC::grpc++ gRPC::grpc CURL::libcurl)

# Generate gRPC files
add_custom_command(
    OUTPUT include/forex.grpc.pb.h include/forex.pb.h
    COMMAND protoc --cpp_out=include proto/forex.proto
    COMMAND grpc_cpp_plugin -I. --cpp_out=include proto/forex.proto
    DEPENDS proto/forex.proto
)

# Install
install(TARGETS forex_service DESTINATION bin)

#Copy nlohmann header files
install(DIRECTORY include_external/nlohmann DESTINATION include)
