# Build Instructions

> **Note:** For quick build, see [README.md](../README.md) for script usage.

## Prerequisites

### Ubuntu 22.04 / Debian 12

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  git \
  libnuma-dev \
  pkg-config
```

### GCC Version

Minimum GCC 12 for C++20 support:

```bash
gcc --version  # Should be 12.0 or higher
```

## Quick Build

```bash
# Clone repository
cd /path/to/hft_elements

# Create build directory
mkdir build && cd build

# Configure (Release mode with optimizations)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build all targets
cmake --build . -j$(nproc)

# Binaries will be in:
# - build/src/simulator/dbn_simulator
# - build/src/server/mbl_server
# - build/src/collector/mbl_collector
```

## Build Configurations

### Debug Build

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j$(nproc)
```

Features:
- Debug symbols (`-g`)
- No optimization (`-O0`)
- Assertions enabled
- Useful for development and debugging

### Release Build

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

Features:
- Maximum optimization (`-O3`)
- AMD-specific tuning (`-march=native`)
- Assertions disabled (`-DNDEBUG`)
- Use for production and benchmarking

### Release with Debug Info

```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build . -j$(nproc)
```

Features:
- Optimizations enabled
- Debug symbols included
- Useful for profiling

## Build with Tests

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON ..
cmake --build . -j$(nproc)

# Run tests
ctest --output-on-failure
```

## Static Analysis

### Clang-Tidy

```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
clang-tidy src/**/*.cpp --
```

### CppCheck

```bash
cppcheck --enable=all --std=c++20 src/
```

## Building with Docker

### Build Image

```bash
cd docker
docker build -t hft-builder:latest -f Dockerfile.build ..
```

### Runtime Image

```bash
# First build with build image
docker build -t hft-runtime:latest -f Dockerfile.runtime ..
```

### Full Stack

```bash
docker-compose up --build
```

## Cross-compilation

For other architectures (e.g., ARM):

```bash
# Install cross-compiler
sudo apt-get install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# Configure for ARM64
cmake -DCMAKE_SYSTEM_NAME=Linux \
      -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
      -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
      -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
      ..
```

## Compiler Flags

### Optimization Flags (automatically set)

- `-march=native`: Use all CPU instructions available
- `-mtune=native`: Tune for current CPU
- `-O3`: Maximum optimization
- `-DNDEBUG`: Disable assertions

### Warning Flags

- `-Wall`: All warnings
- `-Wextra`: Extra warnings
- `-Wpedantic`: ISO C++ compliance

### Performance Flags

You can add additional flags:

```bash
cmake -DCMAKE_CXX_FLAGS="-flto -ffast-math" ..
```

Options:
- `-flto`: Link-time optimization
- `-ffast-math`: Fast floating-point math
- `-fno-omit-frame-pointer`: Better profiling

## Troubleshooting

### "numa.h not found"

```bash
sudo apt-get install libnuma-dev
```

### "CMake version too old"

```bash
# Install newer CMake
wget https://github.com/Kitware/CMake/releases/download/v3.27.0/cmake-3.27.0-linux-x86_64.sh
chmod +x cmake-3.27.0-linux-x86_64.sh
sudo ./cmake-3.27.0-linux-x86_64.sh --prefix=/usr/local --skip-license
```

### "Cannot find -lnuma"

```bash
sudo ldconfig
```

### Build errors with GCC < 12

Upgrade to GCC 12:

```bash
sudo apt-get install gcc-12 g++-12
export CC=gcc-12
export CXX=g++-12
```

## Cleaning Build

```bash
cd build
rm -rf *
cmake ..
cmake --build . -j$(nproc)
```

Or start fresh:

```bash
rm -rf build
mkdir build && cd build
cmake ..
```

