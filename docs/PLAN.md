# Implementation Plan: HFT MBO-to-MBL Feed Conversion System

This document outlines the implementation plan for the HFT elements project, specifically the MBO-to-MBL feed conversion system with real-time JSON snapshot generation.

---

## 🏗️ Core Architecture: Library-First Design

**Key Requirement**: All functionality must be implemented as **reusable libraries** with clean, testable interfaces. Executables are thin wrappers that compose these libraries.

### Library Architecture:

| Library | Purpose | Public Interface |
|---------|---------|------------------|
| `libhft_common` | NUMA utils, CPU pinning, timing, memory pools | `NumaAllocator`, `CpuPinner`, `RdtscTimer` |
| `libhft_dbn` | DBN file parsing and event deserialization | `DbnReader`, `MboEvent` |
| `libhft_mbo_mbl` | Lock-free order book (MBO→MBL) | `OrderBook`, `PriceLevel` |
| `libhft_snapshot` | JSON snapshot generation with caching | `SnapshotGenerator`, `JsonCache` |
| `libhft_network` | Epoll TCP server/client | `TcpServer`, `TcpClient`, `EpollLoop` |
| `libhft_metrics` | Prometheus metrics collection | `LatencyHistogram`, `MetricsRegistry` |

### Benefits:
- ✅ **Testability**: Each library can be unit tested independently
- ✅ **Reusability**: Libraries can be used in different executables
- ✅ **Modularity**: Clear separation of concerns
- ✅ **Mockability**: Easy to mock dependencies for testing
- ✅ **Benchmarking**: Libraries can be benchmarked in isolation

---

## 📋 Phase 1: Project Setup & Infrastructure

### Architecture Principle: **Library-First Design**
All core functionality must be implemented as **reusable libraries** with clean interfaces. Executables are thin wrappers around these libraries for ease of testing and modularity.

### Tasks:
- **CMake Build System Setup**
  - **Library targets** (static libraries only):
    - `libhft_common.a` - Common utilities, NUMA, CPU pinning
    - `libhft_dbn.a` - DBN file reader and parser
    - `libhft_mbo_mbl.a` - MBO to MBL converter (lock-free)
    - `libhft_snapshot.a` - MBL to JSON snapshot generator
    - `libhft_network.a` - TCP server/client with epoll
    - `libhft_metrics.a` - Prometheus metrics collection
  - **Executable targets** (thin wrappers):
    - `dbn_simulator` - Uses libhft_dbn + libhft_network
    - `mbl_server` - Uses libhft_mbo_mbl + libhft_snapshot + libhft_network
    - `mbl_collector` - Uses libhft_network
  - **Test targets**: Unit tests for each library
  - Compiler flags for AMD optimization (`-march=native`, `-mtune=native`)
  - C++20 standard enforcement
  - Dependencies: databento, nlohmann/json or simdjson, prometheus-cpp for metrics
  
- **Project Structure**
  ```
  hft_elements/
  ├── CMakeLists.txt                    # Root CMake
  ├── cmake/                            # CMake modules
  ├── include/
  │   └── hft_elements/
  │       ├── atomic_primitive.h        # Already exists
  │       ├── common/                   # Common utilities headers
  │       ├── dbn/                      # DBN parser headers
  │       ├── mbo_mbl/                  # MBO->MBL converter headers
  │       ├── snapshot/                 # Snapshot generator headers
  │       ├── network/                  # Network library headers
  │       └── metrics/                  # Metrics library headers
  ├── src/
  │   ├── common/                       # libhft_common implementation
  │   ├── dbn/                          # libhft_dbn implementation
  │   ├── mbo_mbl/                      # libhft_mbo_mbl implementation
  │   ├── snapshot/                     # libhft_snapshot implementation
  │   ├── network/                      # libhft_network implementation
  │   ├── metrics/                      # libhft_metrics implementation
  │   ├── simulator/                    # dbn_simulator executable
  │   ├── server/                       # mbl_server executable
  │   └── collector/                    # mbl_collector executable
  ├── tests/
  │   ├── unit/                         # Unit tests per library
  │   │   ├── test_mbo_mbl.cpp
  │   │   ├── test_snapshot.cpp
  │   │   ├── test_network.cpp
  │   │   └── ...
  │   └── integration/                  # End-to-end tests
  ├── docker/
  └── grafana/
  ```

- **Development Environment**
  - Docker build image with AMD-optimized toolchain
  - NUMA library integration (`libnuma-dev`)
  - CPU pinning utilities
  
### Library Design Principles:
- **Static linking only** - All libraries are static (.a) for:
  - Better compiler optimization (whole program optimization)
  - No dynamic linking overhead at runtime
  - Simpler deployment (single binary)
  - Better performance for HFT requirements
- **NO VIRTUAL FUNCTIONS** - Zero virtual dispatch overhead:
  - Use templates for compile-time polymorphism
  - Use CRTP (Curiously Recurring Template Pattern) where needed
  - No vtable lookups in hot path
  - All function calls should be directly resolved at compile time
- **Header-only where appropriate** (templates, inline functions)
- **Pure interfaces** - No global state in libraries
- **Dependency injection** - Libraries take dependencies as constructor/template parameters
- **Testable** - All libraries have comprehensive unit tests
- **Mockable** - Use templates and compile-time dependency injection for testing

### CMake Library Structure Example:
```cmake
# Root CMakeLists.txt

# Build all static libraries
add_subdirectory(src/common)       # Builds libhft_common.a
add_subdirectory(src/dbn)          # Builds libhft_dbn.a
add_subdirectory(src/mbo_mbl)      # Builds libhft_mbo_mbl.a
add_subdirectory(src/snapshot)     # Builds libhft_snapshot.a
add_subdirectory(src/network)      # Builds libhft_network.a
add_subdirectory(src/metrics)      # Builds libhft_metrics.a

# Each library CMakeLists.txt uses:
# add_library(hft_common STATIC ${SOURCES})

# Executables depend on static libraries
add_subdirectory(src/simulator)    # Links: libhft_dbn.a, libhft_network.a
add_subdirectory(src/server)       # Links: libhft_mbo_mbl.a, libhft_snapshot.a, libhft_network.a
add_subdirectory(src/collector)    # Links: libhft_network.a

# Tests
add_subdirectory(tests/unit)       # Links: libraries individually for unit tests
add_subdirectory(tests/integration) # Links: all libraries for integration tests
```

---

## 📡 Phase 2: DBN Simulator & TCP Feed

### Components:
1. **DBN File Reader**
   - Parse Databento binary format (MBO events)
   - Efficient memory-mapped file reading
   - Event deserialization

2. **Rate Controller**
   - Configurable replay rate (CLI parameter: `--rate <events/sec>`)
   - Timestamp-aware replay or max-speed mode
   - Backpressure detection

3. **TCP Server (Simulator Output)**
   - Single producer of binary MBO events
   - Binary protocol over TCP (port configurable)
   - Handle client disconnection gracefully

### Key Considerations:
- Use `io_uring` or `sendmsg` for zero-copy sends
- Pre-allocate buffers to avoid allocation in hot path

---

## ⚡ Phase 3: MBO to MBL Converter (Lock-free)

### Core Algorithm:
- **Order Book Maintenance**
  - Hash map for order ID → Order details (price, size, side)
  - Price level aggregation (price → total size per side)
  - Lock-free data structures (e.g., `folly::ConcurrentHashMap` or custom)

### Lock-free Implementation:
- **Single-writer, multiple-reader pattern**
  - Writer thread: processes incoming MBO events
  - Reader threads: snapshot generator reads current state
  - Use atomic operations and memory ordering (`std::memory_order_acquire/release`)

- **Wait-free Level Updates**
  - Pre-allocated price level array (configurable range)
  - Atomically update aggregated sizes

### Data Structures:
```cpp
struct Order {
    uint64_t order_id;
    uint32_t price;  // Fixed-point representation
    uint32_t size;
    uint8_t side;    // Bid=0, Ask=1
};

struct PriceLevel {
    std::atomic<uint32_t> total_size;
    uint32_t price;
};
```

### Optimizations:
- CPU pinning for MBO processor thread
- NUMA-aware memory allocation
- Cache-line alignment (64 bytes) to avoid false sharing
- Prefetching for hash lookups

---

## 📸 Phase 4: MBL Snapshot Generator with JSON Caching

### JSON Caching Strategy:
1. **Pre-rendered JSON Fragments**
   - Cache serialized price levels: `{"price": "100.50", "size": "1000"}`
   - Reuse unchanged levels between snapshots
   - Dirty flag per price level

2. **Incremental Building**
   - Maintain "last snapshot" state
   - Compute diff between current and last
   - Replace only modified levels in JSON string

3. **Memory Pool**
   - Pre-allocate JSON buffers (e.g., 4KB per snapshot)
   - Ring buffer for snapshot storage

### Implementation:
- Use `simdjson` or custom serializer (faster than nlohmann/json)
- Format: 
```json
{
  "symbol": "AAPL",
  "timestamp": 1234567890123456,
  "bids": [[100.50, 1000], [100.49, 500]],
  "asks": [[100.51, 800], [100.52, 1200]]
}
```

### Latency Optimization:
- Pre-compute string representations
- Avoid dynamic allocations in hot path
- Use `sprintf` or custom int-to-string for numbers

---

## 🌐 Phase 5: TCP Server with Epoll & Client Management

### Architecture:
1. **Epoll Event Loop**
   - Edge-triggered mode for efficiency
   - Non-blocking sockets
   - Handle EPOLLIN (new connections), EPOLLOUT (send ready), EPOLLHUP (disconnect)

2. **Client State Management**
   - Per-client buffer (ring buffer)
   - Track "last sent snapshot" sequence number
   - On new connection: send latest complete snapshot immediately

3. **Backpressure Handling**
   - If client buffer full: drop updates (per requirements)
   - Log dropped snapshots for monitoring
   - Priority: accuracy + latency > guaranteed delivery

### Threading Model:
- **Thread 1**: MBO processor (reads from simulator)
- **Thread 2**: Snapshot generator (converts MBL to JSON)
- **Thread 3**: Epoll server (sends JSON to clients)
- Lock-free queues between threads (e.g., `boost::lockfree::spsc_queue`)

### Key Features:
- TCP_NODELAY for low latency
- SO_RCVBUF/SO_SNDBUF tuning
- Configurable listen port (default: 9550)

---

## 💾 Phase 6: Collector Client

### Functionality:
- Connect to TCP server
- Receive JSON snapshots
- Write to disk with rotation:
  - File naming: `snapshots_YYYYMMDD_HHMMSS.jsonl`
  - Rotation every N snapshots or time interval
  - Compression option (gzip)

### Performance:
- Buffered writes (batch multiple snapshots)
- Async I/O to avoid blocking receive path

---

## 📊 Phase 7: Metrics & Monitoring (P95 Latency)

### Latency Measurement:
1. **Hop-by-hop**:
   - `T1`: Simulator sends MBO event (embed timestamp in message)
   - `T2`: MBO processor finishes update
   - `T3`: Snapshot generated
   - `T4`: Sent to TCP client

2. **End-to-end**:
   - `T1` → `T5` (client receives snapshot)

### Metrics Collection:
- Use **Prometheus client library** (prometheus-cpp)
- Histograms for latency (P50, P95, P99, max)
- Counters: events processed, snapshots sent, clients connected, drops
- Gauges: current order book depth, active connections

### Instrumentation:
- RDTSC for nanosecond precision timing
- Minimal overhead in hot path (update metrics in separate thread)

---

## 🐳 Phase 8: Docker & Grafana Dashboard

### Docker Images:
1. **Build Image** (`hft-builder:latest`)
   - Ubuntu 22.04 + GCC 12 + CMake
   - All dependencies pre-installed
   - Reproducible builds

2. **Runtime Image** (`hft-simulator:latest`, `hft-server:latest`, `hft-collector:latest`)
   - Minimal base (distroless or alpine)
   - Copy binaries from build stage
   - NUMA libraries included

### Docker Compose Setup:
```yaml
services:
  simulator:
    image: hft-simulator:latest
    ports: ["9550:9550"]  # MBO feed
    volumes: ["./data:/data"]  # DBN files
    
  mbl_server:
    image: hft-server:latest
    ports: ["9551:9551"]  # MBL JSON feed
    
  prometheus:
    image: prom/prometheus
    ports: ["9552:9090"]
    
  grafana:
    image: grafana/grafana
    ports: ["9553:3000"]
```

### Grafana Dashboard:
- **Panel 1**: Latency histogram (P95 line chart)
- **Panel 2**: Throughput (events/sec)
- **Panel 3**: Active connections
- **Panel 4**: Drop rate
- **Panel 5**: CPU usage per component

---

## 🚀 Phase 9: Performance Optimization & Testing

### Optimization Checklist:
- [ ] Profile with `perf` to identify hotspots
- [ ] Verify NUMA placement (`numactl --cpunodebind=0`)
- [ ] Cache-line alignment verification
- [ ] False sharing detection (perf c2c)
- [ ] Branch prediction optimization (likely/unlikely macros)
- [ ] SIMD for price level aggregation (AVX2/AVX512)

### Testing Strategy (Library-First):

1. **Unit Tests** (per library):
   - `test_mbo_mbl`: Order book operations, lock-free correctness
   - `test_snapshot`: JSON caching, incremental updates
   - `test_network`: Connection handling, backpressure
   - `test_dbn`: DBN parsing, event deserialization
   - `test_metrics`: Latency measurement accuracy
   - Use Google Test or Catch2 framework
   - Mock I/O dependencies for isolated testing

2. **Library Benchmarks**:
   - Microbenchmarks for each library's critical path
   - Measure overhead of library abstractions
   - Verify lock-free progress guarantees

3. **Integration Tests**:
   - End-to-end flow with sample DBN file
   - Multi-client scenarios
   - Failure recovery testing

4. **Load Tests**:
   - 1M+ events/sec throughput
   - Sustained load over hours
   - Memory leak detection (valgrind)

5. **Latency Tests**:
   - Verify P95 < target (e.g., 1µs for MBO→MBL)
   - Per-library latency breakdown
   - Regression testing

---

## 🎯 Success Criteria

- ✅ No dropped MBO events from simulator
- ✅ 1:1 snapshot generation under normal load
- ✅ Graceful degradation (drop snapshots, not events) under backpressure
- ✅ P95 hop-hop latency < 5µs (configurable target)
- ✅ P95 end-end latency < 50µs
- ✅ Support 10+ concurrent collector clients
- ✅ Reproducible Docker builds
- ✅ Real-time Grafana monitoring

---

## 📦 Deliverables

1. **Static Library Collection** (C++20, CMake):
   - `libhft_common.a` - Shared utilities
   - `libhft_dbn.a` - DBN parser
   - `libhft_mbo_mbl.a` - Order book engine
   - `libhft_snapshot.a` - JSON snapshot generator
   - `libhft_network.a` - TCP networking
   - `libhft_metrics.a` - Metrics collection
   - Header files with clean public APIs
   - All libraries built as static archives (.a)

2. **Executables**:
   - `dbn_simulator` - DBN file replay
   - `mbl_server` - MBL snapshot server
   - `mbl_collector` - Snapshot collector client

3. **Test Suite**:
   - Unit tests for all libraries
   - Integration tests
   - Benchmarking suite
   - Test data samples

4. **Docker Images**:
   - Build image (reproducible builds)
   - Runtime images per executable
   - docker-compose.yml for demo

5. **Monitoring**:
   - Grafana dashboard JSON
   - Prometheus configuration

6. **Documentation**:
   - README with setup instructions
   - API documentation (Doxygen)
   - Performance benchmarks report
   - Library usage examples

---

## 🔧 Technical Stack

- **Language**: C++20
- **Build System**: CMake
- **Concurrency**: Lock-free/wait-free data structures, SeqLock
- **Networking**: Epoll, TCP with TCP_NODELAY
- **Serialization**: simdjson or custom JSON serializer
- **Metrics**: Prometheus client library
- **Containerization**: Docker, Docker Compose
- **Monitoring**: Grafana + Prometheus
- **CPU Optimization**: NUMA APIs, CPU pinning, cache-line alignment
- **Timing**: RDTSC for nanosecond precision

---

## 📝 Notes

- The implementation prioritizes **accuracy and latency** over guaranteed delivery of every snapshot
- **Lock-free/wait-free** patterns are used throughout to minimize contention
- **SeqLock** implementation (available in `include/hft_elements/common/atomic_primitive.h`) will be used for single-writer, multiple-reader scenarios
- All components are designed for **AMD CPU optimization** with appropriate compiler flags
- **False sharing** prevention through cache-line alignment (64 bytes)
- Port range for external services: **9500-9600** (per project conventions)

## 📚 Additional Documentation

- **[ARCHITECTURE.md](ARCHITECTURE.md)** - Detailed system architecture and threading model
- **[BUILD.md](BUILD.md)** - Build instructions and troubleshooting
- **[QUICKREF.md](QUICKREF.md)** - Quick reference cheat sheet
- **[PROJECT.md](PROJECT.md)** - Original project requirements
- **[../.cursorrules](../.cursorrules)** - Critical implementation guardrails

