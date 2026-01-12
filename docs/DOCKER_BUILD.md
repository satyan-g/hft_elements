# Docker Build Guide

## Overview

This project uses Docker for reproducible builds with a consistent toolchain. Binaries built in Docker are **statically linked** and work on the host system without additional dependencies.

## Build Environment

**Docker Container:**
- OS: Ubuntu 22.04 LTS
- glibc: 2.35
- GCC: 12.3.0
- CMake: 3.22.1
- Static linking: libstdc++, libgcc

**Host System (Example):**
- OS: Ubuntu 22.04 LTS
- glibc: 2.35
- GCC: 11.4.0
- CMake: 3.22.1

**Result:** Docker-built binaries run directly on host - no version conflicts!

## Quick Start

### Build in Docker

```bash
# Build release version
./scripts/build.sh release --docker

# Clean build
./scripts/build.sh release --docker --clean

# Debug build
./scripts/build.sh debug --docker
```

### Run Built Binaries on Host

```bash
# Binaries are in build/<type>/
./build/release/examples/utilities/hft_host_diag/hft_host_diag
```

## Build Workflow

1. **Source:** Mounted from host (`-v $(pwd):/workspace`)
2. **Build:** Happens in Docker container
3. **Output:** Written to host's `build/` directory
4. **Run:** Directly on host (no Docker needed)

## Advantages

### Why Docker Build?

✅ **Reproducible:** Same toolchain every time  
✅ **Newer GCC:** 12.3 vs host's 11.4  
✅ **Clean environment:** No conflicts with host packages  
✅ **Team consistency:** Everyone uses same build environment  
✅ **CI/CD ready:** Same Dockerfile for CI pipelines  

### Why Not Just Use Host?

❌ Host toolchain varies by developer  
❌ Upgrading system GCC can break other software  
❌ Harder to reproduce build issues  

## Static Linking

Binaries statically link C++ runtime but dynamically link system libraries:

**Statically Linked (portable):**
- libstdc++ (C++ standard library)
- libgcc (GCC runtime)

**Dynamically Linked (system):**
- libc (glibc 2.35)
- libnuma (NUMA support)
- libpthread (threading)

### Verify Static Linking

```bash
ldd ./build/release/examples/utilities/hft_host_diag/hft_host_diag
```

Should **NOT** show:
- ❌ libstdc++.so.6
- ❌ libgcc_s.so.1

Should show:
- ✅ libc.so.6
- ✅ libnuma.so.1

## Docker Image Management

### Rebuild Docker Image

```bash
docker build -f docker/Dockerfile.build -t hft-build:latest .
```

### List Images

```bash
docker images | grep hft
```

### Remove Old Images

```bash
# Remove specific image
docker rmi hft-build:latest

# Remove dangling images
docker image prune
```

## Troubleshooting

### Docker Build Fails

```bash
# Check Docker is running
docker ps

# Clean Docker build cache
docker system prune -a
```

### Binary Won't Run on Host

Check glibc compatibility:

```bash
# Host glibc version
ldd --version

# Binary requirements
ldd ./build/release/examples/utilities/hft_host_diag/hft_host_diag
```

Both should have same glibc major.minor version.

### Permission Issues

If files created by Docker are owned by root:

```bash
# Docker runs as your user (see build.sh)
docker run -u "$(id -u):$(id -g)" ...
```

## Performance Notes

### First Build
- Downloads Ubuntu base image (~50MB)
- Installs packages (~200MB)
- Takes ~2-3 minutes

### Subsequent Builds
- Uses cached Docker layers
- Only compiles changed source
- Takes ~10-30 seconds

### Build Cache Location

Docker cache: `/var/lib/docker/`  
Build artifacts: `./build/` (on host, mounted)

## Integration with IDE

### VS Code / Cursor

Use local build for development:

```bash
./scripts/build.sh debug
```

Use Docker build for release:

```bash
./scripts/build.sh release --docker
```

### CMake in IDE

Point to local build directory:

```
Build Directory: ${workspaceFolder}/build/debug
```

## CI/CD Integration

```yaml
# .github/workflows/build.yml
- name: Build in Docker
  run: ./scripts/build.sh release --docker
  
- name: Run tests
  run: ./build/release/tests/run_tests
```

## Philosophy

> **"Keep host stable, innovate in Docker"**

- Host: Ubuntu 22.04 LTS, minimal changes
- Docker: Latest toolchain, reproducible builds
- Result: Best of both worlds - stability + modern tools

