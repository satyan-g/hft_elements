#ifndef HFT_ELEMENTS_SNAPSHOT_JSON_BUILDER_H
#define HFT_ELEMENTS_SNAPSHOT_JSON_BUILDER_H

#include "hft_elements/common/types.h"
#include "hft_elements/mbo_mbl/order_book.h"
#include <string>
#include <string_view>
#include <array>

namespace hft_elements {
namespace snapshot {

// Fast JSON builder with caching for price levels
// No allocations in hot path after initialization
class JsonBuilder {
public:
  static constexpr size_t MAX_SNAPSHOT_SIZE = 8192;  // 8KB max per snapshot
  
  JsonBuilder() noexcept;
  
  // Build JSON snapshot from order book levels
  // Returns string_view into internal buffer
  // Buffer is valid until next build() call
  std::string_view build(
    const common::Symbol& symbol,
    common::Timestamp timestamp,
    const std::array<mbo_mbl::PriceLevel, mbo_mbl::OrderBook::MAX_LEVELS>& bids,
    const std::array<mbo_mbl::PriceLevel, mbo_mbl::OrderBook::MAX_LEVELS>& asks,
    size_t bid_count,
    size_t ask_count) noexcept;
  
  // Pre-build cached representations for common prices/quantities
  void warm_cache() noexcept;

private:
  // Internal buffer for building JSON
  std::array<char, MAX_SNAPSHOT_SIZE> buffer_;
  
  // Helper: append price to buffer
  char* append_price(char* dst, common::Price price) noexcept;
  
  // Helper: append quantity to buffer
  char* append_quantity(char* dst, common::Quantity qty) noexcept;
  
  // Helper: append timestamp
  char* append_timestamp(char* dst, common::Timestamp ts) noexcept;
  
  // Helper: append symbol
  char* append_symbol(char* dst, const common::Symbol& symbol) noexcept;
};

} // namespace snapshot
} // namespace hft_elements

#endif

