# Quick Reference - HFT Implementation Checklist

## ✅ DO

```cpp
// ✅ Use strong types
void process(Price price, Quantity qty, OrderId id);

// ✅ Cache-line align hot structures
struct alignas(64) PriceLevel { /* ... */ uint8_t padding[N]; };
static_assert(sizeof(PriceLevel) == 64);

// ✅ Use SeqLock for sync
atomic_primitive::SeqWriteGuard guard(lock_);

// ✅ Template polymorphism
template<typename T> class Processor { /* ... */ };

// ✅ Pre-allocate buffers
std::array<PriceLevel, MAX> levels_;

// ✅ Pin threads
CpuUtils::pin_to_core(core_id);

// ✅ Relaxed atomics for counters
counter_.fetch_add(1, std::memory_order_relaxed);

// ✅ Inline hot functions
inline Price get_best() const noexcept { return levels_[0].price; }

// ✅ Branch hints
if (UNLIKELY(error)) { handle_error(); }

// ✅ Pass by const ref
void process(const Event& event) noexcept;

// ✅ Mark noexcept
constexpr Price midpoint(Price a, Price b) noexcept;

// ✅ Add metrics
auto start = RdtscTimer::now();
/* ... */
histogram_.record(RdtscTimer::now() - start);
```

## ❌ DON'T

```cpp
// ❌ NO primitive types
void process(uint32_t price, uint32_t qty);  // Type confusion!

// ❌ NO unaligned hot data
struct Level { uint32_t p, q; };  // False sharing!

// ❌ NO mutex in hot path
std::lock_guard<std::mutex> lock(m_);  // 100s of ns overhead!

// ❌ NO virtual functions
virtual void process() = 0;  // Vtable lookup!

// ❌ NO allocation in hot path
std::vector<Level> v; v.push_back(l);  // Heap alloc!

// ❌ NO unpinned threads
std::thread t([](){ process(); });  // NUMA migration!

// ❌ NO default seq_cst
counter_.fetch_add(1);  // Too expensive!

// ❌ NO non-inline hot calls
Price get_best();  // Function call overhead!

// ❌ NO unhinted branches
if (error) { /* ... */ }  // Misprediction penalty!

// ❌ NO pass by value
void process(Event e);  // Copies 64 bytes!

// ❌ NO missing noexcept
Price midpoint(Price a, Price b);  // Exception overhead!

// ❌ NO unmeasured code
process_event(e);  // Can't optimize blind!
```

## 📏 Size Requirements

```cpp
static_assert(sizeof(PriceLevel) == 64, "Cache line");
static_assert(sizeof(MboEvent) == 64, "Cache line");
static_assert(alignof(PriceLevel) == 64, "Aligned");
```

## 🔧 Common Patterns

### SeqLock (Single Writer, Multiple Readers)

```cpp
// Writer
{
  SeqWriteGuard g(lock_);
  modify_data();
}

// Reader
for (;;) {
  SeqReadAttempt a(lock_);
  if (!a.can_read()) continue;
  read_data();
  if (a.validate()) break;
}
```

### Memory Ordering

```cpp
// Counters (no ordering)
count_.fetch_add(1, std::memory_order_relaxed);

// Producer-Consumer
flag_.store(true, std::memory_order_release);  // Producer
if (flag_.load(std::memory_order_acquire)) {}  // Consumer
```

### NUMA Setup

```cpp
CpuUtils::pin_to_core(core);
int node = CpuUtils::get_numa_node(core);
void* mem = CpuUtils::numa_alloc(size, node);
```

### Pre-allocated JSON

```cpp
class Builder {
  std::array<char, 8192> buf_;
  
  std::string_view build() {
    char* p = buf_.data();
    p = append(p, data);
    return {buf_.data(), p};
  }
};
```

## 🎯 Performance Targets

- **Cache miss**: ~100 ns
- **Mutex lock**: ~50-100 ns
- **Virtual call**: ~5-10 ns  
- **Atomic (relaxed)**: ~1-2 ns
- **Inline call**: ~0 ns

**Target: P95 < 5 µs end-to-end**

## 📚 Key Files

- `.cursorrules` - Full implementation guide
- `include/hft_elements/common/types.h` - Type definitions
- `include/hft_elements/common/atomic_primitive.h` - SeqLock
- `docs/ARCHITECTURE.md` - System design
- `docs/PLAN.md` - Implementation plan

