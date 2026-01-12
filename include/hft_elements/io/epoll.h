#ifndef HFT_ELEMENTS_IO_EPOLL_H
#define HFT_ELEMENTS_IO_EPOLL_H

#include "hft_elements/io/socket.h"
#include <cstdint>
#include <functional>
#include <sys/epoll.h>

namespace hft_elements {
namespace io {

// Epoll event types
enum class EpollEvent : uint32_t {
  Read = EPOLLIN,
  Write = EPOLLOUT,
  Error = EPOLLERR,
  HangUp = EPOLLHUP,
  EdgeTriggered = EPOLLET
};

// Epoll wrapper for event-driven I/O
class Epoll {
public:
  static constexpr int MAX_EVENTS = 1024;
  
  Epoll() noexcept;
  ~Epoll() noexcept;
  
  // Non-copyable, movable
  Epoll(const Epoll&) = delete;
  Epoll& operator=(const Epoll&) = delete;
  Epoll(Epoll&& other) noexcept;
  Epoll& operator=(Epoll&& other) noexcept;
  
  // Create epoll instance
  bool create() noexcept;
  
  // Add socket to epoll with events
  bool add(int fd, uint32_t events, void* user_data = nullptr) noexcept;
  
  // Modify socket events
  bool modify(int fd, uint32_t events, void* user_data = nullptr) noexcept;
  
  // Remove socket from epoll
  bool remove(int fd) noexcept;
  
  // Wait for events (timeout in milliseconds, -1 = block forever)
  // Returns number of events
  int wait(int timeout_ms = -1) noexcept;
  
  // Get event at index (call after wait())
  const epoll_event& get_event(int index) const noexcept;
  
  // Close epoll
  void close() noexcept;
  
  // Check if valid
  bool is_valid() const noexcept { return epoll_fd_ >= 0; }

private:
  int epoll_fd_{-1};
  epoll_event events_[MAX_EVENTS];
  int event_count_{0};
};

} // namespace io
} // namespace hft_elements

#endif

