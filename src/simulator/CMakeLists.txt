# dbn_simulator executable
add_executable(dbn_simulator
  main.cpp
)

target_link_libraries(dbn_simulator PRIVATE
  hft_dbn
  hft_network
  hft_metrics
  hft_common
)

