#ifndef HFT_ELEMENTS_NETWORK_TCP_CLIENT_H
#define HFT_ELEMENTS_NETWORK_TCP_CLIENT_H

#include "hft_elements/network/socket.h"
#include <string>
#include <cstdint>
#include <functional>

namespace hft_elements {
namespace network {

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

} // namespace network
} // namespace hft_elements

#endif

