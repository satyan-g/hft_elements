# libhft_common - Common utilities
add_library(hft_common STATIC
  # Implementation files will go here
  # types.cpp
  # timing.cpp
  # cpu.cpp
  # memory.cpp
)

target_include_directories(hft_common PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(hft_common PUBLIC
  numa
  pthread
)

