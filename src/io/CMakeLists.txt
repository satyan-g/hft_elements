# libhft_io - Network I/O with epoll
add_library(lib_hft_io STATIC
  placeholder.cpp
  # socket.cpp
  # epoll.cpp
  # tcp_server.cpp (if needed, mostly header-only template)
  # tcp_client.cpp
)

target_include_directories(lib_hft_io PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(lib_hft_io PUBLIC
  lib_hft_common
)

