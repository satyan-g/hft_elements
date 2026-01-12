#ifndef HFT_ELEMENTS_FEEDS_EVENTS_H
#define HFT_ELEMENTS_FEEDS_EVENTS_H

#include "hft_elements/common/types.h"
#include <cstdint>

namespace hft_elements {
namespace feeds {

// MBO event types (simplified, based on Databento schema)
enum class EventType : uint8_t {
  Add = 1,
  Cancel = 2,
  Modify = 3,
  Trade = 4,
  Clear = 5
};

// MBO event structure (cache-line aligned)
struct alignas(64) MboEvent {
  common::Timestamp timestamp;
  common::OrderId order_id;
  common::Price price;
  common::Quantity quantity;
  common::Symbol symbol;
  common::Side side;
  EventType event_type;
  uint8_t padding[7];  // Align to 64 bytes
  
  MboEvent() noexcept;
};

static_assert(sizeof(MboEvent) == 64, "MboEvent must be cache-line sized");

} // namespace feeds
} // namespace hft_elements

#endif

