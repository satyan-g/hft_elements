# libhft_dbn - DBN file reader
add_library(hft_dbn STATIC
  # Implementation files will go here
  # events.cpp
  # reader.cpp
  # rate_limiter.cpp
)

target_include_directories(hft_dbn PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(hft_dbn PUBLIC
  hft_common
)

