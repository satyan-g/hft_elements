#ifndef HFT_ELEMENTS_FEEDS_READER_H
#define HFT_ELEMENTS_FEEDS_READER_H

#include "hft_elements/feeds/events.h"
#include <string>
#include <cstdint>

namespace hft_elements {
namespace feeds {

// DBN file reader with memory-mapped I/O
class DbnReader {
public:
  explicit DbnReader(const std::string& filepath);
  ~DbnReader() noexcept;
  
  // Non-copyable, movable
  DbnReader(const DbnReader&) = delete;
  DbnReader& operator=(const DbnReader&) = delete;
  DbnReader(DbnReader&&) noexcept;
  DbnReader& operator=(DbnReader&&) noexcept;
  
  // Open file and memory map
  bool open();
  
  // Close file
  void close() noexcept;
  
  // Read next event (returns false at EOF)
  bool read_next(MboEvent& event) noexcept;
  
  // Rewind to beginning
  void rewind() noexcept;
  
  // Get total number of events
  uint64_t total_events() const noexcept;
  
  // Check if EOF
  bool eof() const noexcept;

private:
  std::string filepath_;
  void* mapped_data_{nullptr};
  size_t file_size_{0};
  size_t current_offset_{0};
  uint64_t event_count_{0};
  bool is_open_{false};
};

} // namespace feeds
} // namespace hft_elements

#endif

