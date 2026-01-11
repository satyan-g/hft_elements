# Architecture Overview

## System Design

```
┌─────────────────┐     TCP      ┌──────────────────┐     TCP       ┌──────────────┐
│  dbn_simulator  │─────────────>│   mbl_server     │──────────────>│ mbl_collector│
│                 │  MBO Events  │                  │  JSON Snaps   │              │
│  libhft_dbn     │              │  3 Threads:      │               │ libhft_net   │
│  libhft_network │              │  - MBO processor │               │              │
└─────────────────┘              │  - Snapshot gen  │               └──────────────┘
                                 │  - TCP server    │
                                 │                  │
                                 │  libhft_mbo_mbl  │
                                 │  libhft_snapshot │
                                 │  libhft_network  │
                                 └──────────────────┘
```

## Threading Model

### mbl_server (3 threads)

1. **MBO Processor Thread** (CPU core pinned)
   - Connects to simulator
   - Receives binary MBO events
   - Updates OrderBook (lock-free writes)
   - Metrics: MBO processing latency (T1→T2)

2. **Snapshot Generator Thread** (CPU core pinned)
   - Reads OrderBook state (lock-free reads via SeqLock)
   - Generates JSON snapshots with caching
   - Pushes to broadcast queue
   - Metrics: Snapshot generation latency (T2→T3)

3. **TCP Server Thread** (CPU core pinned)
   - Epoll event loop
   - Accepts client connections
   - Broadcasts snapshots to all clients
   - Handles backpressure (drops updates if client buffer full)
   - Metrics: Send latency (T3→T4), clients connected, drops

## Lock-free Synchronization

### SeqLock Pattern

Located in `include/hft_elements/common/atomic_primitive.h`

```cpp
// Writer (MBO processor)
{
  SeqWriteGuard guard(book.lock);
  // Modify order book
} // Commit on scope exit

// Reader (snapshot generator)
for (;;) {
  SeqReadAttempt attempt(book.lock);
  if (!attempt.can_read()) continue;
  
  // Read book state
  
  if (attempt.validate()) break;
  // Retry if concurrent write detected
}
```

### Benefits
- No mutex overhead
- Writer never blocks
- Reader retries on conflict (rare in practice)
- CPU cache-friendly (no lock contention)

## Memory Layout

### Cache-line Alignment

All hot-path structures are 64-byte aligned to prevent false sharing:

```cpp
struct alignas(64) PriceLevel {
  Price price;
  Quantity quantity;
  bool is_dirty;
  uint8_t padding[55];  // Total = 64 bytes
};
```

### NUMA Awareness

- Pin threads to specific CPU cores
- Allocate memory on local NUMA node
- Prefetch data before use

## Data Flow

```
MBO Event → OrderBook Update → MBL Snapshot → JSON Cache → TCP Send → Client
   T1           T2                 T3             T3          T4        T5

Latency tracking:
- Hop 1: T1→T2 (MBO processing)
- Hop 2: T2→T3 (Snapshot generation)
- Hop 3: T3→T4 (TCP send)
- End-to-end: T1→T5
```

## Backpressure Handling

Per requirements: **Accuracy + Latency > Guaranteed Delivery**

### Strategy
1. Never drop MBO events (must process all)
2. Generate 1:1 snapshots under normal load
3. If client send buffer full:
   - Drop snapshot for that client
   - Log drop event
   - Continue serving other clients
4. Client sees latest complete snapshot on reconnect

### Metrics
- `snapshots_generated`: Total snapshots created
- `snapshots_sent`: Successfully sent
- `snapshots_dropped`: Dropped due to backpressure
- `clients_connected`: Current client count

## Performance Optimizations

### Compile-time
- `-march=native`: Use all available CPU instructions (AVX2, AVX512)
- `-O3`: Aggressive optimization
- Static linking: Whole program optimization
- No virtual functions: Direct function calls only

### Runtime
- CPU pinning: Avoid context switches
- NUMA allocation: Local memory access
- Busy waiting: No syscall overhead for queues
- Prefetching: Hide memory latency
- Cache-line alignment: Avoid false sharing

### JSON Generation
- Pre-rendered strings for common values
- Incremental building (only changed levels)
- No dynamic allocation in hot path
- Custom int-to-string (faster than std::to_string)

## Testing Strategy

### Unit Tests
- Each library tested independently
- Mock dependencies via templates
- Fast feedback loop

### Integration Tests
- End-to-end with real data
- Multi-client scenarios
- Failure recovery

### Benchmarks
- Microbenchmarks per library
- Latency distribution (P50, P95, P99, max)
- Throughput under load

### Load Tests
- Sustained 1M+ events/sec
- 10+ concurrent clients
- Hours of continuous operation
- Memory leak detection

