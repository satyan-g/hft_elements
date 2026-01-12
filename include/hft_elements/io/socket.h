#ifndef HFT_ELEMENTS_IO_SOCKET_H
#define HFT_ELEMENTS_IO_SOCKET_H

#include <cstdint>
#include <string>
#include <sys/socket.h>

namespace hft_elements {
namespace io {

// RAII socket wrapper
class Socket {
public:
  Socket() noexcept;
  explicit Socket(int fd) noexcept;
  ~Socket() noexcept;
  
  // Non-copyable, movable
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;
  Socket(Socket&& other) noexcept;
  Socket& operator=(Socket&& other) noexcept;
  
  // Create socket
  bool create(int domain, int type, int protocol) noexcept;
  
  // Bind to address
  bool bind(const std::string& ip, uint16_t port) noexcept;
  
  // Listen for connections
  bool listen(int backlog = 128) noexcept;
  
  // Accept connection (returns new socket)
  Socket accept() noexcept;
  
  // Connect to server
  bool connect(const std::string& ip, uint16_t port) noexcept;
  
  // Send data
  ssize_t send(const void* data, size_t size, int flags = 0) noexcept;
  
  // Receive data
  ssize_t recv(void* buffer, size_t size, int flags = 0) noexcept;
  
  // Set non-blocking
  bool set_nonblocking(bool enable) noexcept;
  
  // Set TCP_NODELAY
  bool set_nodelay(bool enable) noexcept;
  
  // Set SO_REUSEADDR
  bool set_reuseaddr(bool enable) noexcept;
  
  // Set socket buffer sizes
  bool set_rcvbuf(int size) noexcept;
  bool set_sndbuf(int size) noexcept;
  
  // Close socket
  void close() noexcept;
  
  // Get file descriptor
  int fd() const noexcept { return fd_; }
  
  // Check if valid
  bool is_valid() const noexcept { return fd_ >= 0; }

private:
  int fd_{-1};
};

} // namespace io
} // namespace hft_elements

#endif

