#ifndef HFT_ELEMENTS_COMMON_TYPES_H
#define HFT_ELEMENTS_COMMON_TYPES_H

#include <cstdint>
#include <cstddef>

namespace hft_elements {
namespace common {

// Fixed-point price representation (4 decimal places)
// e.g., 100.50 = 1005000
using Price = uint32_t;

// Quantity/size type
using Quantity = uint32_t;

// Order ID type
using OrderId = uint64_t;

// Timestamp in nanoseconds
using Timestamp = uint64_t;

// Symbol representation (fixed 8 chars for cache efficiency)
struct alignas(8) Symbol {
  char data[8];
  
  Symbol() noexcept : data{} {}
  explicit Symbol(const char* str) noexcept;
  
  bool operator==(const Symbol& other) const noexcept;
  bool operator!=(const Symbol& other) const noexcept;
};

// Side enum
enum class Side : uint8_t {
  Bid = 0,
  Ask = 1
};

// Constants
constexpr Price PRICE_MULTIPLIER = 10000;  // 4 decimal places
constexpr size_t CACHE_LINE_SIZE = 64;

} // namespace common
} // namespace hft_elements

#endif


