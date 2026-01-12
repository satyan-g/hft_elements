# libhft_orderbook - Generic order book
add_library(lib_hft_orderbook STATIC
  placeholder.cpp
  # book.cpp
  # book_manager.cpp
)

target_include_directories(lib_hft_orderbook PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(lib_hft_orderbook PUBLIC
  lib_hft_common
  lib_hft_feeds
)

