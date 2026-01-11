#ifndef HFT_ELEMENTS_METRICS_HISTOGRAM_H
#define HFT_ELEMENTS_METRICS_HISTOGRAM_H

#include <cstdint>
#include <array>
#include <string>

namespace hft_elements {
namespace metrics {

// Fast lock-free histogram for latency measurements
// Uses fixed buckets for lock-free updates
class Histogram {
public:
  // Bucket boundaries in nanoseconds
  static constexpr size_t NUM_BUCKETS = 64;
  static constexpr uint64_t MAX_VALUE = 1000000000;  // 1 second
  
  Histogram() noexcept;
  
  // Record a value (nanoseconds)
  void record(uint64_t value_ns) noexcept;
  
  // Get percentile (e.g., 95 for P95)
  uint64_t percentile(double p) const noexcept;
  
  // Get mean
  double mean() const noexcept;
  
  // Get min/max
  uint64_t min() const noexcept;
  uint64_t max() const noexcept;
  
  // Get total count
  uint64_t count() const noexcept;
  
  // Reset histogram
  void reset() noexcept;
  
  // Export to Prometheus format
  std::string to_prometheus(const std::string& name) const;

private:
  std::array<uint64_t, NUM_BUCKETS> buckets_;
  uint64_t total_count_{0};
  uint64_t sum_{0};
  uint64_t min_value_{MAX_VALUE};
  uint64_t max_value_{0};
  
  size_t value_to_bucket(uint64_t value) const noexcept;
  uint64_t bucket_to_value(size_t bucket) const noexcept;
};

} // namespace metrics
} // namespace hft_elements

#endif

