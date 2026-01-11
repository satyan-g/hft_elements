# mbl_collector executable
add_executable(mbl_collector
  main.cpp
)

target_link_libraries(mbl_collector PRIVATE
  hft_network
  hft_common
)

