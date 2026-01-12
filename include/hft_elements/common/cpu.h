#ifndef HFT_ELEMENTS_COMMON_CPU_H
#define HFT_ELEMENTS_COMMON_CPU_H

#include <cstdint>

namespace hft_elements {
namespace common {

// CPU affinity and NUMA utilities
class CpuUtils {
public:
  // Pin current thread to specific CPU core
  static bool pin_to_core(int core_id) noexcept;
  
  // Get NUMA node for a given CPU core
  static int get_numa_node(int core_id) noexcept;
  
  // Allocate memory on specific NUMA node
  static void* numa_alloc(size_t size, int node) noexcept;
  
  // Free NUMA-allocated memory
  static void numa_free(void* ptr, size_t size) noexcept;
  
  // Prefetch cache line
  static inline void prefetch(const void* ptr) noexcept;
  
  // Compiler fence (prevent reordering)
  static inline void compiler_barrier() noexcept;
};

} // namespace common
} // namespace hft_elements

#endif


