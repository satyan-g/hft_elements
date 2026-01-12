#ifndef HFT_ELEMENTS_FEEDS_RATE_LIMITER_H
#define HFT_ELEMENTS_FEEDS_RATE_LIMITER_H

#include <cstdint>

namespace hft_elements {
namespace feeds {

// Token bucket rate limiter for replay control
class RateLimiter {
public:
  // events_per_second: target rate (0 = unlimited)
  explicit RateLimiter(uint64_t events_per_second) noexcept;
  
  // Wait until allowed to send next event
  void wait() noexcept;
  
  // Update rate (events per second)
  void set_rate(uint64_t events_per_second) noexcept;
  
  // Get current rate
  uint64_t get_rate() const noexcept;
  
  // Reset limiter state
  void reset() noexcept;

private:
  uint64_t events_per_second_;
  uint64_t last_timestamp_;
  double tokens_;
  static constexpr double BUCKET_SIZE = 1000.0;  // Burst capacity
};

} // namespace feeds
} // namespace hft_elements

#endif

