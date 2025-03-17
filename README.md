# Forex gRPC Service (C++)

This is a C++ gRPC service that fetches forex rates from the Open Exchange Rates API.

## Prerequisites

* gRPC C++
* CURL
* nlohmann/json
* CMake
* protoc

## Build Instructions

1.  Create the `build` directory: `mkdir build && cd build`
2.  Run CMake: `cmake ..`
3.  Build: `make`
4.  Run: `./forex_service`

## Environment Variable

Set the `FOREX_API_KEY` environment variable with your Open Exchange Rates API key.

```bash
export FOREX_API_KEY="YOUR_API_KEY"