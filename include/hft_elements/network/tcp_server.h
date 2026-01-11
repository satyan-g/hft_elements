#ifndef HFT_ELEMENTS_NETWORK_TCP_SERVER_H
#define HFT_ELEMENTS_NETWORK_TCP_SERVER_H

#include "hft_elements/network/socket.h"
#include "hft_elements/network/epoll.h"
#include "hft_elements/common/memory.h"
#include <cstdint>
#include <unordered_map>
#include <string_view>
#include <functional>

namespace hft_elements {
namespace network {

// Client connection state
struct ClientConnection {
  Socket socket;
  common::RingBuffer<char, 65536> send_buffer;  // 64KB per client
  uint64_t bytes_sent{0};
  uint64_t bytes_dropped{0};
  bool writable{true};
  
  explicit ClientConnection(Socket&& sock) noexcept
    : socket(std::move(sock)) {}
};

// TCP server with epoll (template for snapshot provider dependency injection)
template<typename SnapshotProvider>
class TcpServer {
public:
  using ClientCallback = std::function<void(int client_fd)>;
  
  explicit TcpServer(uint16_t port, SnapshotProvider& provider) noexcept
    : port_(port), snapshot_provider_(provider) {}
  
  ~TcpServer() noexcept {
    stop();
  }
  
  // Start server
  bool start() noexcept;
  
  // Stop server
  void stop() noexcept;
  
  // Run event loop (blocking, call from server thread)
  void run() noexcept;
  
  // Broadcast data to all connected clients
  // Returns number of clients successfully sent to
  size_t broadcast(std::string_view data) noexcept;
  
  // Get number of connected clients
  size_t client_count() const noexcept { return clients_.size(); }
  
  // Set callback for new connections
  void set_connect_callback(ClientCallback cb) noexcept {
    on_connect_ = std::move(cb);
  }
  
  // Set callback for disconnections
  void set_disconnect_callback(ClientCallback cb) noexcept {
    on_disconnect_ = std::move(cb);
  }

private:
  uint16_t port_;
  SnapshotProvider& snapshot_provider_;
  Socket listen_socket_;
  Epoll epoll_;
  std::unordered_map<int, ClientConnection> clients_;
  bool running_{false};
  
  ClientCallback on_connect_;
  ClientCallback on_disconnect_;
  
  // Event handlers
  void handle_new_connection() noexcept;
  void handle_client_event(int fd, uint32_t events) noexcept;
  void handle_client_read(int fd) noexcept;
  void handle_client_write(int fd) noexcept;
  void disconnect_client(int fd) noexcept;
  
  // Send latest snapshot to new client
  void send_initial_snapshot(int fd) noexcept;
};

} // namespace network
} // namespace hft_elements

#endif

