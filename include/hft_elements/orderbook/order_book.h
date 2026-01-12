#ifndef HFT_ELEMENTS_ORDERBOOK_ORDER_BOOK_H
#define HFT_ELEMENTS_ORDERBOOK_ORDER_BOOK_H

#include "hft_elements/common/types.h"
#include "hft_elements/common/atomic_primitive.h"
#include "hft_elements/feeds/events.h"
#include <array>
#include <unordered_map>

namespace hft_elements {
namespace orderbook {

// Single price level (aggregated quantity at a price)
struct alignas(64) PriceLevel {
  common::Price price;
  common::Quantity quantity;
  bool is_dirty;  // Changed since last snapshot
  uint8_t padding[55];
  
  PriceLevel() noexcept : price(0), quantity(0), is_dirty(false), padding{} {}
};

static_assert(sizeof(PriceLevel) == 64, "PriceLevel must be cache-line sized");

// Order information (for MBO tracking)
struct Order {
  common::OrderId order_id;
  common::Price price;
  common::Quantity quantity;
  common::Side side;
  
  Order() noexcept : order_id(0), price(0), quantity(0), side(common::Side::Bid) {}
};

// Lock-free order book (single writer, multiple readers)
// Uses SeqLock for synchronization
class OrderBook {
public:
  static constexpr size_t MAX_LEVELS = 256;  // Max price levels per side
  
  explicit OrderBook(const common::Symbol& symbol) noexcept;
  
  // Process MBO event (writer thread only)
  void process_event(const feeds::MboEvent& event) noexcept;
  
  // Get current book state (reader thread)
  // Returns false if concurrent write detected (retry)
  bool try_read_levels(
    std::array<PriceLevel, MAX_LEVELS>& bids,
    std::array<PriceLevel, MAX_LEVELS>& asks,
    size_t& bid_count,
    size_t& ask_count) const noexcept;
  
  // Get symbol
  const common::Symbol& symbol() const noexcept { return symbol_; }
  
  // Clear all orders
  void clear() noexcept;

private:
  common::Symbol symbol_;
  
  // SeqLock for synchronization
  atomic_primitive::SeqLock lock_;
  
  // Active orders (order_id -> Order)
  std::unordered_map<common::OrderId, Order> orders_;
  
  // Price levels (aggregated view)
  std::array<PriceLevel, MAX_LEVELS> bid_levels_;
  std::array<PriceLevel, MAX_LEVELS> ask_levels_;
  size_t bid_count_{0};
  size_t ask_count_{0};
  
  // Helper methods
  void handle_add(const feeds::MboEvent& event) noexcept;
  void handle_cancel(const feeds::MboEvent& event) noexcept;
  void handle_modify(const feeds::MboEvent& event) noexcept;
  void handle_clear() noexcept;
  
  void rebuild_levels() noexcept;
  size_t find_level(const std::array<PriceLevel, MAX_LEVELS>& levels, 
                    size_t count, common::Price price) const noexcept;
};

} // namespace orderbook
} // namespace hft_elements

#endif

