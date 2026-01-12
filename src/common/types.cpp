#include "hft_elements/common/types.h"
#include <cstring>

namespace hft_elements {
namespace common {

Symbol::Symbol(const char* str) noexcept : data{} {
  if (str) {
    std::strncpy(data, str, sizeof(data) - 1);
  }
}

bool Symbol::operator==(const Symbol& other) const noexcept {
  return std::memcmp(data, other.data, sizeof(data)) == 0;
}

bool Symbol::operator!=(const Symbol& other) const noexcept {
  return !(*this == other);
}

} // namespace common
} // namespace hft_elements

