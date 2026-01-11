#ifndef HFT_ELEMENTS_COMMON_MEMORY_H
#define HFT_ELEMENTS_COMMON_MEMORY_H

#include <cstdint>
#include <cstddef>
#include <array>

namespace hft_elements {
namespace common {

// Cache-line aligned allocator
template<typename T>
class alignas(64) CacheAlignedAllocator {
public:
  using value_type = T;
  
  T* allocate(size_t n);
  void deallocate(T* ptr, size_t n) noexcept;
};

// Simple ring buffer (lock-free SPSC)
template<typename T, size_t Capacity>
class alignas(64) RingBuffer {
public:
  RingBuffer() noexcept;
  
  // Producer: Try to push an item (returns false if full)
  bool try_push(const T& item) noexcept;
  bool try_push(T&& item) noexcept;
  
  // Consumer: Try to pop an item (returns false if empty)
  bool try_pop(T& item) noexcept;
  
  // Check if empty/full
  bool empty() const noexcept;
  bool full() const noexcept;
  
  size_t size() const noexcept;

private:
  alignas(64) std::array<T, Capacity> buffer_;
  alignas(64) size_t head_{0};  // Producer writes here
  alignas(64) size_t tail_{0};  // Consumer reads here
};

// Memory pool for fixed-size allocations
template<typename T, size_t PoolSize>
class MemoryPool {
public:
  MemoryPool() noexcept;
  ~MemoryPool() noexcept;
  
  // Allocate object (returns nullptr if pool exhausted)
  T* allocate() noexcept;
  
  // Return object to pool
  void deallocate(T* ptr) noexcept;
  
  size_t available() const noexcept;

private:
  alignas(64) std::array<T, PoolSize> pool_;
  T* free_list_{nullptr};
  size_t free_count_{PoolSize};
};

} // namespace common
} // namespace hft_elements

#endif

