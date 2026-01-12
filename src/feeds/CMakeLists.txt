# libhft_feeds - Feed readers (DBN, etc.)
add_library(lib_hft_feeds STATIC
  placeholder.cpp
  # dbn_reader.cpp
  # rate_limiter.cpp
  # feed_types.cpp
)

target_include_directories(lib_hft_feeds PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(lib_hft_feeds PUBLIC
  lib_hft_common
)

