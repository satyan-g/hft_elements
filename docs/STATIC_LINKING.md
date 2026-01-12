# Static Linking Guide

This project uses static linking for C++ runtime libraries to ensure portability and optimal performance.

## What is Statically Linked

### Currently Enabled:
```cmake
-static-libgcc -static-libstdc++
```

This means:
- ✅ **libgcc** (GCC runtime) - statically linked
- ✅ **libstdc++** (C++ standard library) - statically linked
- ✅ Binaries built with GCC 14 in Docker run on Ubuntu 22.04 host
- ✅ No dependency on host system's libstdc++ version

### Still Dynamically Linked:
- **glibc** (system C library) - dynamically linked
- **libnuma** - dynamically linked
- **pthread** - dynamically linked

This is intentional - these are system libraries that should match the host.

## Fully Static Linking (Optional)

To build fully static binaries, uncomment in `CMakeLists.txt`:

```cmake
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
```

### Pros:
- Binary runs on any Linux system (even Alpine Linux)
- Zero external dependencies
- Maximum portability

### Cons:
- Much larger binaries
- Cannot use NSS (name service switch)
- Requires static versions of all libraries (`libnuma-dev` has static lib)

## Verifying Static Linking

Check what's dynamically linked:

```bash
ldd ./build/release/examples/utilities/hft_host_diag/hft_host_diag
```

With `-static-libgcc -static-libstdc++`, you should see:
```
linux-vdso.so.1
libnuma.so.1 => /usr/lib/x86_64-linux-gnu/libnuma.so.1
libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0
libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6
/lib64/ld-linux-x86-64.so.2
```

Notice: **NO libstdc++.so.6** in the list!

With full `-static`, you should see:
```
not a dynamic executable
```

## Why This Matters for HFT

### Performance Benefits:
1. **No PLT/GOT overhead**: Direct function calls, no indirection
2. **Better inlining**: Compiler sees all code at link time
3. **Faster startup**: No dynamic linker resolution
4. **Better cache locality**: All code in one place

### Portability Benefits:
1. **Version independence**: GCC 14 binary runs on GCC 11 system
2. **Deployment simplicity**: Copy binary, it just works
3. **Docker portability**: Build once, run anywhere

## Build Examples

### Local build (uses host GCC 11):
```bash
./scripts/build.sh release
# Binaries use system libstdc++ (dynamic)
```

### Docker build (uses GCC 14):
```bash
docker build -f docker/Dockerfile.build -t hft-builder .
docker run --rm -v $(pwd):/src hft-builder
# Binaries have GCC 14 libstdc++ statically linked
```

### Check binary compatibility:
```bash
file ./build/release/examples/utilities/hft_host_diag/hft_host_diag
strings ./build/release/examples/utilities/hft_host_diag/hft_host_diag | grep GLIBCXX
```

If you see `GLIBCXX_3.4.32` in the binary but it still runs on Ubuntu 22.04, that means static linking is working!

## Troubleshooting

### Error: cannot find -lstdc++
Install static library:
```bash
sudo apt-get install libstdc++-14-dev
```

### Error: undefined reference to `numa_xxx`
Install static numa:
```bash
sudo apt-get install libnuma-dev
```

### Binary still dynamically links libstdc++
Check CMakeLists.txt has:
```cmake
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
```

And rebuild from scratch:
```bash
rm -rf build && ./scripts/build.sh release
```

