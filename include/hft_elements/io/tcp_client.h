#ifndef HFT_ELEMENTS_IO_TCP_CLIENT_H
#define HFT_ELEMENTS_IO_TCP_CLIENT_H

#include "hft_elements/io/socket.h"
#include <string>
#include <cstdint>
#include <functional>

namespace hft_elements {
namespace io {

// TCP client for receiving snapshots
class TcpClient {
public:
  using DataCallback = std::function<void(const char* data, size_t size)>;
  
  TcpClient(const std::string& host, uint16_t port) noexcept;
  ~TcpClient() noexcept;
  
  // Connect to server
  bool connect() noexcept;
  
  // Disconnect from server
  void disconnect() noexcept;
  
  // Receive data (blocking, call from receiver thread)
  // Returns number of bytes received, 0 on disconnect, -1 on error
  ssize_t receive(char* buffer, size_t size) noexcept;
  
  // Run receive loop with callback
  // Calls callback for each received chunk
  void run_receive_loop(DataCallback callback) noexcept;
  
  // Check if connected
  bool is_connected() const noexcept;

private:
  std::string host_;
  uint16_t port_;
  Socket socket_;
  bool connected_{false};
};

} // namespace io
} // namespace hft_elements

#endif

