# HFT Elements - Reusable High-Performance Trading Components

A collection of reusable, high-performance libraries for building HFT (High-Frequency Trading) applications. Designed with a library-first approach where components are generic and applications are built by composing these libraries.

## Architecture

### Generic Reusable Libraries

| Library | Description | Use Cases |
|---------|-------------|-----------|
| `libhft_common.a` | Core utilities | NUMA, CPU pinning, timing (RDTSC), memory pools, SeqLock |
| `libhft_feeds.a` | Feed readers | DBN parsing, rate limiting - extensible to other feed formats |
| `libhft_orderbook.a` | Order book engine | Lock-free order book maintenance, MBO/MBL aggregation |
| `libhft_serialization.a` | Data serialization | JSON generation with caching - extensible to other formats |
| `libhft_io.a` | Network I/O | Epoll-based TCP server/client, non-blocking I/O |
| `libhft_metrics.a` | Performance metrics | Latency histograms (P95), counters, gauges |

### Example Applications

**Utilities** (`examples/utilities/`):
- **`hft_host_diag`**: HFT host configuration diagnostic tool
- **`dbncat`**: DBN file parser and JSON converter (uses Databento C++ library)

**MBO to MBL Converter** (`examples/mbo_to_mbl/`):
- **`feed_simulator`**: Replays DBN files over TCP
- **`book_server`**: Maintains order books and serves JSON snapshots
- **`data_collector`**: Collects and saves snapshots to disk

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
# Run individual components (MBO to MBL example)
./scripts/run.sh feed_simulator --file data/sample.dbn --port 9550
./scripts/run.sh book_server --mbo-host localhost --mbo-port 9550
./scripts/run.sh data_collector --host localhost --port 9551

# Run all components together
./scripts/run.sh all

# Run debug build
./scripts/run.sh debug feed_simulator --help
```

### Manual Run

### 1. Feed Simulator

Replay DBN file over TCP:

```bash
./build/release/examples/mbo_to_mbl/feed_simulator/feed_simulator \
  --file data/sample.dbn \
  --port 9550 \
  --rate 100000 \
  --core 0
```

### 2. Book Server

Convert MBO to MBL and serve snapshots:

```bash
./build/release/examples/mbo_to_mbl/book_server/book_server \
  --mbo-host localhost \
  --mbo-port 9550 \
  --snapshot-port 9551 \
  --mbo-core 1 \
  --snapshot-core 2 \
  --server-core 3
```

### 3. Data Collector

Receive and save snapshots:

```bash
./build/release/examples/mbo_to_mbl/data_collector/data_collector \
  --host localhost \
  --port 9551 \
  --output ./snapshots \
  --rotate 100000 \
  --compress
```

## Design Philosophy

- **Library-first**: Generic, reusable components, not tied to specific applications
- **Static linking**: C++ runtime statically linked for portability and performance
- **Zero-cost abstractions**: Templates instead of virtual functions
- **Lock-free/wait-free**: SeqLock for synchronization, no mutexes in hot path
- **Cache-optimized**: 64-byte alignment, false sharing prevention
- **Type-safe**: Strong type aliases, no primitive types in interfaces
- **NUMA-aware**: CPU pinning, local memory allocation

### Static Linking Benefits

Binaries built with GCC 14 in Docker run on Ubuntu 22.04+ hosts without requiring newer libstdc++:
- ✅ No dynamic linking overhead (PLT/GOT indirection)
- ✅ Better compiler optimizations (whole program optimization)
- ✅ Portable binaries across Linux distributions
- ✅ See `docs/STATIC_LINKING.md` for details

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
├── include/hft_elements/       # Public library headers
│   ├── common/                 # Core utilities, NUMA, timing, SeqLock
│   ├── feeds/                  # Feed readers (DBN, etc.)
│   ├── orderbook/              # Generic order book
│   ├── serialization/          # JSON and other serializers
│   ├── io/                     # Network I/O (TCP, epoll)
│   └── metrics/                # Performance metrics
├── src/                        # Library implementations
│   ├── common/                 # libhft_common.a
│   ├── feeds/                  # libhft_feeds.a
│   ├── orderbook/              # libhft_orderbook.a
│   ├── serialization/          # libhft_serialization.a
│   ├── io/                     # libhft_io.a
│   └── metrics/                # libhft_metrics.a
├── examples/                   # Example applications
│   └── mbo_to_mbl/             # MBO-to-MBL converter example
│       ├── feed_simulator/     # Feed replay
│       ├── book_server/        # Order book server
│       └── data_collector/     # Data collection client
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
