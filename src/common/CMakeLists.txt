# libhft_common - Common utilities
add_library(lib_hft_common STATIC
  types.cpp
  # timing.cpp
  # cpu.cpp
  # memory.cpp
)

target_include_directories(lib_hft_common PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(lib_hft_common PUBLIC
  numa
  pthread
)


