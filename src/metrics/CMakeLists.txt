# libhft_metrics - Metrics collection
add_library(lib_hft_metrics STATIC
  placeholder.cpp
  # histogram.cpp
  # counter.cpp
  # gauge.cpp
  # registry.cpp
)

target_include_directories(lib_hft_metrics PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(lib_hft_metrics PUBLIC
  lib_hft_common
)


