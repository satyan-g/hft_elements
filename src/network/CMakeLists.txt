# libhft_network - TCP networking with epoll
add_library(hft_network STATIC
  # Implementation files will go here
  # socket.cpp
  # epoll.cpp
  # tcp_server.cpp (if needed, mostly header-only template)
  # tcp_client.cpp
)

target_include_directories(hft_network PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(hft_network PUBLIC
  hft_common
)

