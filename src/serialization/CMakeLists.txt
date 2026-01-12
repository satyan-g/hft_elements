# libhft_serialization - JSON and other serializers
add_library(lib_hft_serialization STATIC
  placeholder.cpp
  # json_builder.cpp
  # serializer.cpp (if needed, mostly header-only)
)

target_include_directories(lib_hft_serialization PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(lib_hft_serialization PUBLIC
  lib_hft_common
  lib_hft_orderbook
)

