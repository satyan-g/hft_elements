# libhft_snapshot - JSON snapshot generator
add_library(hft_snapshot STATIC
  # Implementation files will go here
  # json_builder.cpp
  # generator.cpp (if needed, mostly header-only)
)

target_include_directories(hft_snapshot PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(hft_snapshot PUBLIC
  hft_common
  hft_mbo_mbl
)

