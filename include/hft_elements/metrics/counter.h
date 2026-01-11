#ifndef HFT_ELEMENTS_METRICS_COUNTER_H
#define HFT_ELEMENTS_METRICS_COUNTER_H

#include <atomic>
#include <cstdint>
#include <string>

namespace hft_elements {
namespace metrics {

// Simple atomic counter
class Counter {
public:
  Counter() noexcept : value_(0) {}
  
  // Increment counter
  void increment(uint64_t delta = 1) noexcept {
    value_.fetch_add(delta, std::memory_order_relaxed);
  }
  
  // Get current value
  uint64_t value() const noexcept {
    return value_.load(std::memory_order_relaxed);
  }
  
  // Reset counter
  void reset() noexcept {
    value_.store(0, std::memory_order_relaxed);
  }
  
  // Export to Prometheus format
  std::string to_prometheus(const std::string& name) const;

private:
  std::atomic<uint64_t> value_;
};

} // namespace metrics
} // namespace hft_elements

#endif

