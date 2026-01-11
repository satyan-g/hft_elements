# HFT Elements - MBO to MBL Feed Conversion System

High-performance market data processing system that converts Market-By-Order (MBO) feeds to Market-By-Level (MBL) feeds with JSON snapshot generation.

## Architecture

This project follows a **library-first design** with all core functionality implemented as static libraries. Executables are thin wrappers around these libraries.

### Libraries

| Library | Description | Key Features |
|---------|-------------|--------------|
| `libhft_common.a` | Common utilities | NUMA, CPU pinning, timing (RDTSC), memory pools |
| `libhft_dbn.a` | DBN file parsing | Memory-mapped file reading, rate limiting |
| `libhft_mbo_mbl.a` | Order book engine | Lock-free MBO→MBL conversion with SeqLock |
| `libhft_snapshot.a` | JSON snapshot generator | Incremental JSON building with caching |
| `libhft_network.a` | TCP networking | Epoll-based server/client, non-blocking I/O |
| `libhft_metrics.a` | Metrics collection | Histograms (P95), counters, gauges |

### Executables

- **`dbn_simulator`**: Replays DBN files over TCP with configurable rate limiting
- **`mbl_server`**: Converts MBO feed to MBL snapshots and serves over TCP
- **`mbl_collector`**: Connects to server and saves snapshots to disk

## Design Principles

- ✅ **Zero virtual functions** - No vtable overhead, compile-time polymorphism
- ✅ **Static linking only** - Better optimization, simpler deployment
- ✅ **Lock-free/wait-free** - SeqLock for single-writer, multiple-reader
- ✅ **Cache-optimized** - 64-byte alignment, false sharing prevention
- ✅ **AMD-optimized** - `-march=native`, NUMA-aware allocation
- ✅ **C++20** - Modern C++ features, concepts, ranges

## Building

### Requirements

- GCC 12+ or Clang 15+
- CMake 3.20+
- libnuma-dev
- Linux kernel 5.10+ (for io_uring support)

### Quick Build (Recommended)

```bash
# Release build (optimized)
./scripts/build.sh release

# Debug build (with symbols)
./scripts/build.sh debug

# Clean build with tests
./scripts/build.sh release --clean --tests

# See all options
./scripts/build.sh --help
```

### Manual Build

```bash
# Create build directory
mkdir build && cd build

# Configure (Release build with optimizations)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build all targets
cmake --build . -j$(nproc)

# Build with tests
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON ..
cmake --build . -j$(nproc)
ctest --output-on-failure
```

## Usage

### Quick Run (Recommended)

```bash
# Run individual components
./scripts/run.sh simulator --file data/sample.dbn --port 9550
./scripts/run.sh server --mbo-host localhost --mbo-port 9550
./scripts/run.sh collector --host localhost --port 9551

# Run all components together
./scripts/run.sh all

# Run debug build
./scripts/run.sh debug simulator --help
```

### Manual Run

### 1. DBN Simulator

Replay DBN file over TCP:

```bash
./build/release/src/simulator/dbn_simulator \
  --file data/sample.dbn \
  --port 9550 \
  --rate 100000 \
  --core 0
```

### 2. MBL Server

Convert MBO to MBL and serve snapshots:

```bash
./build/release/src/server/mbl_server \
  --mbo-host localhost \
  --mbo-port 9550 \
  --snapshot-port 9551 \
  --mbo-core 1 \
  --snapshot-core 2 \
  --server-core 3
```

### 3. Collector

Receive and save snapshots:

```bash
./build/release/src/collector/mbl_collector \
  --host localhost \
  --port 9551 \
  --output ./snapshots \
  --rotate 100000 \
  --compress
```

## Performance Targets

- **P95 hop-hop latency**: < 5µs (MBO event → MBL update)
- **P95 end-to-end latency**: < 50µs (MBO event → client receives snapshot)
- **Throughput**: > 1M events/sec
- **Concurrent clients**: 10+ with no degradation

## Project Structure

```
hft_elements/
├── CMakeLists.txt              # Root build configuration
├── README.md                   # This file
├── .cursorrules                # Implementation guardrails
├── docs/                       # Documentation
│   ├── ARCHITECTURE.md         # System architecture
│   ├── PLAN.md                 # Implementation plan
│   ├── BUILD.md                # Detailed build instructions
│   ├── QUICKREF.md             # Quick reference cheat sheet
│   ├── PROJECT.md              # Original requirements
│   └── SCAFFOLD_SUMMARY.md     # Scaffold overview
├── scripts/                    # Build and run scripts
│   ├── build.sh                # Build automation
│   └── run.sh                  # Run automation
├── include/
│   └── hft_elements/           # Public headers
│       ├── common/             # Common utilities (includes SeqLock)
│       ├── dbn/                # DBN parser
│       ├── mbo_mbl/            # Order book engine
│       ├── snapshot/           # Snapshot generator
│       ├── network/            # TCP networking
│       └── metrics/            # Metrics collection
├── src/
│   ├── common/                 # libhft_common implementation
│   ├── dbn/                    # libhft_dbn implementation
│   ├── mbo_mbl/                # libhft_mbo_mbl implementation
│   ├── snapshot/               # libhft_snapshot implementation
│   ├── network/                # libhft_network implementation
│   ├── metrics/                # libhft_metrics implementation
│   ├── simulator/              # dbn_simulator executable
│   ├── server/                 # mbl_server executable
│   └── collector/              # mbl_collector executable
├── tests/                      # Unit and integration tests
├── docker/                     # Docker build/runtime images
└── data/                       # Sample data files
```

## Development Guidelines

Before implementing features, please read:
- **`.cursorrules`** - Critical implementation guardrails for HFT performance
- **`docs/ARCHITECTURE.md`** - System design and threading model
- **`docs/PLAN.md`** - Detailed implementation plan
- **`docs/QUICKREF.md`** - One-page implementation cheat sheet

Key principles:
- ✅ Use strong types (never primitive types in signatures)
- ✅ Cache-line alignment (64 bytes) to prevent false sharing
- ✅ Lock-free data structures (SeqLock, atomics)
- ✅ No virtual functions in hot path
- ✅ Pre-allocate memory, no dynamic allocation in hot path

## Development Status

- [x] Project scaffold and interfaces
- [x] Build and run scripts
- [x] Implementation guidelines (.cursorrules)
- [ ] Library implementations
- [ ] Unit tests
- [ ] Integration tests
- [ ] Docker images
- [ ] Grafana dashboards
- [ ] Performance benchmarks

## Documentation

- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** - System architecture and threading model
- **[docs/PLAN.md](docs/PLAN.md)** - Detailed 9-phase implementation plan
- **[docs/BUILD.md](docs/BUILD.md)** - Comprehensive build instructions
- **[docs/QUICKREF.md](docs/QUICKREF.md)** - One-page implementation cheat sheet
- **[docs/PROJECT.md](docs/PROJECT.md)** - Original project requirements
- **[.cursorrules](.cursorrules)** - Critical HFT coding standards

## License

TBD

## Contributing

TBD
