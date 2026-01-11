// DBN Simulator - Replays DBN file over TCP
// Thin wrapper around libhft_dbn and libhft_network

#include "hft_elements/dbn/reader.h"
#include "hft_elements/dbn/rate_limiter.h"
#include "hft_elements/network/socket.h"
#include "hft_elements/common/cpu.h"
#include "hft_elements/metrics/registry.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <getopt.h>

struct Config {
  std::string dbn_file;
  uint16_t port = 9550;
  uint64_t rate = 0;  // 0 = unlimited
  int cpu_core = -1;  // -1 = no pinning
};

void print_usage(const char* prog) {
  std::cerr << "Usage: " << prog << " [options]\n"
            << "Options:\n"
            << "  -f, --file FILE       DBN file to replay (required)\n"
            << "  -p, --port PORT       TCP port to listen on (default: 9550)\n"
            << "  -r, --rate RATE       Events per second (0 = unlimited, default: 0)\n"
            << "  -c, --core CORE       Pin to CPU core (default: no pinning)\n"
            << "  -h, --help            Show this help message\n";
}

Config parse_args(int argc, char* argv[]) {
  Config config;
  
  static struct option long_options[] = {
    {"file", required_argument, 0, 'f'},
    {"port", required_argument, 0, 'p'},
    {"rate", required_argument, 0, 'r'},
    {"core", required_argument, 0, 'c'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };
  
  int opt;
  while ((opt = getopt_long(argc, argv, "f:p:r:c:h", long_options, nullptr)) != -1) {
    switch (opt) {
      case 'f':
        config.dbn_file = optarg;
        break;
      case 'p':
        config.port = static_cast<uint16_t>(std::atoi(optarg));
        break;
      case 'r':
        config.rate = std::stoull(optarg);
        break;
      case 'c':
        config.cpu_core = std::atoi(optarg);
        break;
      case 'h':
      default:
        print_usage(argv[0]);
        std::exit(opt == 'h' ? 0 : 1);
    }
  }
  
  if (config.dbn_file.empty()) {
    std::cerr << "Error: DBN file is required\n";
    print_usage(argv[0]);
    std::exit(1);
  }
  
  return config;
}

int main(int argc, char* argv[]) {
  // Parse command line
  auto config = parse_args(argc, argv);
  
  // Pin to CPU core if requested
  if (config.cpu_core >= 0) {
    if (hft_elements::common::CpuUtils::pin_to_core(config.cpu_core)) {
      std::cout << "Pinned to CPU core " << config.cpu_core << "\n";
    } else {
      std::cerr << "Warning: Failed to pin to CPU core " << config.cpu_core << "\n";
    }
  }
  
  std::cout << "DBN Simulator starting...\n"
            << "  File: " << config.dbn_file << "\n"
            << "  Port: " << config.port << "\n"
            << "  Rate: " << (config.rate == 0 ? "unlimited" : std::to_string(config.rate) + " eps") << "\n";
  
  // TODO: Implementation
  // 1. Open DBN file using DbnReader
  // 2. Create TCP server socket
  // 3. Accept connections
  // 4. Replay events with rate limiting
  // 5. Collect metrics
  
  std::cout << "Simulator stub - implementation pending\n";
  
  return 0;
}

