# libhft_mbo_mbl - MBO to MBL converter
add_library(hft_mbo_mbl STATIC
  # Implementation files will go here
  # order_book.cpp
  # book_manager.cpp
)

target_include_directories(hft_mbo_mbl PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(hft_mbo_mbl PUBLIC
  hft_common
  hft_dbn
)

