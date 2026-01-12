#ifndef HFT_ELEMENTS_METRICS_GAUGE_H
#define HFT_ELEMENTS_METRICS_GAUGE_H

#include <atomic>
#include <cstdint>
#include <string>

namespace hft_elements {
namespace metrics {

// Atomic gauge (can go up or down)
class Gauge {
public:
  Gauge() noexcept : value_(0) {}
  
  // Set gauge value
  void set(int64_t value) noexcept {
    value_.store(value, std::memory_order_relaxed);
  }
  
  // Increment gauge
  void increment(int64_t delta = 1) noexcept {
    value_.fetch_add(delta, std::memory_order_relaxed);
  }
  
  // Decrement gauge
  void decrement(int64_t delta = 1) noexcept {
    value_.fetch_sub(delta, std::memory_order_relaxed);
  }
  
  // Get current value
  int64_t value() const noexcept {
    return value_.load(std::memory_order_relaxed);
  }
  
  // Export to Prometheus format
  std::string to_prometheus(const std::string& name) const;

private:
  std::atomic<int64_t> value_;
};

} // namespace metrics
} // namespace hft_elements

#endif


