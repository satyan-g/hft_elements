#ifndef HFT_ELEMENTS_SERIALIZATION_GENERATOR_H
#define HFT_ELEMENTS_SERIALIZATION_GENERATOR_H

#include "hft_elements/serialization/json_builder.h"
#include "hft_elements/orderbook/book_manager.h"
#include "hft_elements/common/timing.h"
#include <string_view>

namespace hft_elements {
namespace serialization {

// Generates JSON snapshots from order books
// Template parameter allows dependency injection for testing
template<typename BookManagerT = orderbook::BookManager>
class SnapshotGenerator {
public:
  explicit SnapshotGenerator(BookManagerT& book_manager) noexcept
    : book_manager_(book_manager) {
    json_builder_.warm_cache();
  }
  
  // Generate snapshot for a symbol
  // Returns empty string_view if book doesn't exist or read failed
  std::string_view generate(const common::Symbol& symbol) noexcept {
    const auto* book = book_manager_.get_book(symbol);
    if (!book) {
      return {};
    }
    
    // Try to read book levels (may fail if concurrent write)
    std::array<orderbook::PriceLevel, orderbook::OrderBook::MAX_LEVELS> bids, asks;
    size_t bid_count, ask_count;
    
    // Retry loop for SeqLock
    for (int retry = 0; retry < 3; ++retry) {
      if (book->try_read_levels(bids, asks, bid_count, ask_count)) {
        // Success - build JSON
        auto timestamp = common::RdtscTimer::now();
        return json_builder_.build(symbol, timestamp, bids, asks, bid_count, ask_count);
      }
    }
    
    // Failed after retries
    return {};
  }
  
  // Generate snapshots for all symbols
  // Callback is called for each snapshot: void callback(symbol, json_view)
  template<typename Callback>
  void generate_all(Callback&& callback) noexcept {
    auto symbols = book_manager_.get_symbols();
    for (const auto& symbol : symbols) {
      auto snapshot = generate(symbol);
      if (!snapshot.empty()) {
        callback(symbol, snapshot);
      }
    }
  }

private:
  BookManagerT& book_manager_;
  JsonBuilder json_builder_;
};

} // namespace serialization
} // namespace hft_elements

#endif

