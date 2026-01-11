# Documentation Index

This directory contains all technical documentation for the HFT Elements project.

## 📚 Documentation Files

### Getting Started
- **[../README.md](../README.md)** - Main project overview, quick start, and usage
- **[BUILD.md](BUILD.md)** - Detailed build instructions and troubleshooting

### Architecture & Design
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - System architecture, threading model, and data flow
- **[PLAN.md](PLAN.md)** - Detailed 9-phase implementation plan
- **[PROJECT.md](PROJECT.md)** - Original project requirements and specifications

### Implementation
- **[QUICKREF.md](QUICKREF.md)** - One-page implementation cheat sheet (DO/DON'T)
- **[../.cursorrules](../.cursorrules)** - Critical HFT coding standards and guardrails

### Internal
- **[SCAFFOLD_SUMMARY.md](SCAFFOLD_SUMMARY.md)** - Project scaffold overview and statistics

## 🎯 Recommended Reading Order

### For Developers:
1. Start with **[../README.md](../README.md)** for project overview
2. Read **[ARCHITECTURE.md](ARCHITECTURE.md)** to understand the system design
3. Review **[../.cursorrules](../.cursorrules)** for coding standards
4. Use **[QUICKREF.md](QUICKREF.md)** as a quick reference while coding
5. Consult **[PLAN.md](PLAN.md)** for implementation phases

### For Building:
1. **[../README.md](../README.md)** for quick build scripts
2. **[BUILD.md](BUILD.md)** for detailed build instructions

### For Understanding Requirements:
1. **[PROJECT.md](PROJECT.md)** - Original requirements
2. **[PLAN.md](PLAN.md)** - How we plan to implement them
3. **[ARCHITECTURE.md](ARCHITECTURE.md)** - Technical design

## 🔑 Key Concepts

### Performance Critical
- **Type Safety**: Use strong types (Price, Quantity, not uint32_t)
- **Cache Alignment**: 64-byte alignment to prevent false sharing
- **Lock-Free**: SeqLock for synchronization, no mutexes
- **No Virtual Functions**: Template-based polymorphism only
- **Pre-allocation**: No dynamic allocation in hot path

### Threading Model
- **Thread 1**: MBO processor (reads events, updates book)
- **Thread 2**: Snapshot generator (reads book, generates JSON)
- **Thread 3**: TCP server (broadcasts snapshots to clients)

### Performance Targets
- **P95 latency**: < 5µs (hop-to-hop)
- **P95 end-to-end**: < 50µs
- **Throughput**: > 1M events/sec
- **Clients**: 10+ concurrent

## 🔗 External References

- **Libraries**:
  - `include/hft_elements/common/` - Common utilities and types
  - `include/hft_elements/common/atomic_primitive.h` - SeqLock implementation
  
- **Scripts**:
  - `scripts/build.sh` - Build automation
  - `scripts/run.sh` - Run automation

- **Build**:
  - `CMakeLists.txt` - Root build configuration
  - `src/*/CMakeLists.txt` - Per-library build configs

