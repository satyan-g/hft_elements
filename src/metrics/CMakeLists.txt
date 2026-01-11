# libhft_metrics - Metrics collection
add_library(hft_metrics STATIC
  # Implementation files will go here
  # histogram.cpp
  # counter.cpp
  # gauge.cpp
  # registry.cpp
)

target_include_directories(hft_metrics PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(hft_metrics PUBLIC
  hft_common
)

