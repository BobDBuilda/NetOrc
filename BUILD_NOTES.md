# NetOrc Build Notes

## System Dependencies

```bash
sudo apt-get install -y cmake g++ protobuf-compiler libprotobuf-dev \
  libgrpc++-dev libgrpc-dev libsqlite3-dev
```

## Build

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

The build produces three binaries:
- `NetOrc` - The main SDN controller
- `validator_tests` - Unit tests for validation
- `app_integration_tests` - Integration tests

## Build Status (2026-06-01)

Build: SUCCESS

The project built cleanly on Ubuntu 24.04 with g++ 14, CMake 3.30, and
protobuf/gRPC from system packages. The `nlohmann-json3-dev` package is
**not** required — the codebase does not use it.
