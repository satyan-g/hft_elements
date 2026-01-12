#ifndef HFT_ELEMENTS_COMMON_TIMING_H
#define HFT_ELEMENTS_COMMON_TIMING_H

#include <cstdint>

namespace hft_elements {
namespace common {

// RDTSC-based high-resolution timer
class RdtscTimer {
public:
  // Get current timestamp in CPU cycles
  static inline uint64_t now() noexcept;
  
  // Convert cycles to nanoseconds (requires calibration)
  static inline uint64_t cycles_to_ns(uint64_t cycles) noexcept;
  
  // Calibrate the timer (call once at startup)
  static void calibrate() noexcept;

private:
  static uint64_t tsc_frequency_mhz_;
};

// Simple RAII timer for measuring latency
template<typename Callback>
class ScopedTimer {
public:
  explicit ScopedTimer(Callback&& cb) noexcept
    : start_(RdtscTimer::now()), callback_(std::forward<Callback>(cb)) {}
  
  ~ScopedTimer() noexcept {
    uint64_t elapsed = RdtscTimer::now() - start_;
    callback_(elapsed);
  }

private:
  uint64_t start_;
  Callback callback_;
};

} // namespace common
} // namespace hft_elements

#endif


