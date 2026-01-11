#ifndef HFT_ELEMENTS_MBO_MBL_BOOK_MANAGER_H
#define HFT_ELEMENTS_MBO_MBL_BOOK_MANAGER_H

#include "hft_elements/mbo_mbl/order_book.h"
#include "hft_elements/common/types.h"
#include <unordered_map>
#include <memory>

namespace hft_elements {
namespace mbo_mbl {

// Manages multiple order books (one per symbol)
class BookManager {
public:
  BookManager() noexcept;
  ~BookManager() noexcept;
  
  // Process incoming MBO event
  void process_event(const dbn::MboEvent& event) noexcept;
  
  // Get order book for symbol (creates if doesn't exist)
  OrderBook* get_book(const common::Symbol& symbol) noexcept;
  
  // Get order book for symbol (const, returns nullptr if doesn't exist)
  const OrderBook* get_book(const common::Symbol& symbol) const noexcept;
  
  // Get all symbols
  std::vector<common::Symbol> get_symbols() const;
  
  // Clear all books
  void clear_all() noexcept;

private:
  // Symbol -> OrderBook mapping
  std::unordered_map<common::Symbol, std::unique_ptr<OrderBook>> books_;
};

} // namespace mbo_mbl
} // namespace hft_elements

// Hash function for Symbol
namespace std {
  template<>
  struct hash<hft_elements::common::Symbol> {
    size_t operator()(const hft_elements::common::Symbol& s) const noexcept {
      // Simple hash of 8-byte data
      return *reinterpret_cast<const uint64_t*>(s.data);
    }
  };
}

#endif

