# mbl_server executable
add_executable(mbl_server
  main.cpp
)

target_link_libraries(mbl_server PRIVATE
  hft_mbo_mbl
  hft_snapshot
  hft_network
  hft_metrics
  hft_common
  pthread
)

