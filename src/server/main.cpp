// MBL Server - Converts MBO to MBL and serves JSON snapshots
// Thin wrapper around all libraries

#include "hft_elements/dbn/events.h"
#include "hft_elements/mbo_mbl/book_manager.h"
#include "hft_elements/snapshot/generator.h"
#include "hft_elements/network/tcp_server.h"
#include "hft_elements/network/tcp_client.h"
#include "hft_elements/common/cpu.h"
#include "hft_elements/metrics/registry.h"

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <getopt.h>

struct Config {
  std::string mbo_host = "localhost";
  uint16_t mbo_port = 9550;
  uint16_t snapshot_port = 9551;
  int mbo_cpu_core = -1;
  int snapshot_cpu_core = -1;
  int server_cpu_core = -1;
};

void print_usage(const char* prog) {
  std::cerr << "Usage: " << prog << " [options]\n"
            << "Options:\n"
            << "  --mbo-host HOST       MBO feed host (default: localhost)\n"
            << "  --mbo-port PORT       MBO feed port (default: 9550)\n"
            << "  --snapshot-port PORT  Snapshot server port (default: 9551)\n"
            << "  --mbo-core CORE       Pin MBO processor to CPU core\n"
            << "  --snapshot-core CORE  Pin snapshot generator to CPU core\n"
            << "  --server-core CORE    Pin TCP server to CPU core\n"
            << "  -h, --help            Show this help message\n";
}

Config parse_args(int argc, char* argv[]) {
  Config config;
  
  static struct option long_options[] = {
    {"mbo-host", required_argument, 0, 1},
    {"mbo-port", required_argument, 0, 2},
    {"snapshot-port", required_argument, 0, 3},
    {"mbo-core", required_argument, 0, 4},
    {"snapshot-core", required_argument, 0, 5},
    {"server-core", required_argument, 0, 6},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };
  
  int opt;
  while ((opt = getopt_long(argc, argv, "h", long_options, nullptr)) != -1) {
    switch (opt) {
      case 1: config.mbo_host = optarg; break;
      case 2: config.mbo_port = std::atoi(optarg); break;
      case 3: config.snapshot_port = std::atoi(optarg); break;
      case 4: config.mbo_cpu_core = std::atoi(optarg); break;
      case 5: config.snapshot_cpu_core = std::atoi(optarg); break;
      case 6: config.server_cpu_core = std::atoi(optarg); break;
      case 'h':
      default:
        print_usage(argv[0]);
        std::exit(opt == 'h' ? 0 : 1);
    }
  }
  
  return config;
}

int main(int argc, char* argv[]) {
  auto config = parse_args(argc, argv);
  
  std::cout << "MBL Server starting...\n"
            << "  MBO Feed: " << config.mbo_host << ":" << config.mbo_port << "\n"
            << "  Snapshot Port: " << config.snapshot_port << "\n";
  
  // TODO: Implementation
  // 1. Thread 1: Connect to MBO feed, process events -> BookManager
  // 2. Thread 2: Generate snapshots from BookManager
  // 3. Thread 3: TCP server broadcasting snapshots
  // 4. Collect hop-by-hop latency metrics
  
  std::cout << "Server stub - implementation pending\n";
  
  return 0;
}

