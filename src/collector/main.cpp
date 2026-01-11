// MBL Collector - Receives JSON snapshots and saves to disk
// Thin wrapper around libhft_network

#include "hft_elements/network/tcp_client.h"
#include "hft_elements/common/cpu.h"

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <getopt.h>

struct Config {
  std::string host = "localhost";
  uint16_t port = 9551;
  std::string output_dir = ".";
  size_t rotate_count = 100000;  // Rotate after N snapshots
  bool compress = false;
  int cpu_core = -1;
};

void print_usage(const char* prog) {
  std::cerr << "Usage: " << prog << " [options]\n"
            << "Options:\n"
            << "  -h, --host HOST       Server host (default: localhost)\n"
            << "  -p, --port PORT       Server port (default: 9551)\n"
            << "  -o, --output DIR      Output directory (default: .)\n"
            << "  -r, --rotate COUNT    Rotate after N snapshots (default: 100000)\n"
            << "  -z, --compress        Compress output with gzip\n"
            << "  -c, --core CORE       Pin to CPU core\n"
            << "  --help                Show this help message\n";
}

Config parse_args(int argc, char* argv[]) {
  Config config;
  
  static struct option long_options[] = {
    {"host", required_argument, 0, 'h'},
    {"port", required_argument, 0, 'p'},
    {"output", required_argument, 0, 'o'},
    {"rotate", required_argument, 0, 'r'},
    {"compress", no_argument, 0, 'z'},
    {"core", required_argument, 0, 'c'},
    {"help", no_argument, 0, 1},
    {0, 0, 0, 0}
  };
  
  int opt;
  while ((opt = getopt_long(argc, argv, "h:p:o:r:zc:", long_options, nullptr)) != -1) {
    switch (opt) {
      case 'h': config.host = optarg; break;
      case 'p': config.port = std::atoi(optarg); break;
      case 'o': config.output_dir = optarg; break;
      case 'r': config.rotate_count = std::stoull(optarg); break;
      case 'z': config.compress = true; break;
      case 'c': config.cpu_core = std::atoi(optarg); break;
      case 1:
      default:
        print_usage(argv[0]);
        std::exit(opt == 1 ? 0 : 1);
    }
  }
  
  return config;
}

int main(int argc, char* argv[]) {
  auto config = parse_args(argc, argv);
  
  // Pin to CPU core if requested
  if (config.cpu_core >= 0) {
    if (hft_elements::common::CpuUtils::pin_to_core(config.cpu_core)) {
      std::cout << "Pinned to CPU core " << config.cpu_core << "\n";
    }
  }
  
  std::cout << "MBL Collector starting...\n"
            << "  Server: " << config.host << ":" << config.port << "\n"
            << "  Output: " << config.output_dir << "\n"
            << "  Rotate: " << config.rotate_count << " snapshots\n"
            << "  Compress: " << (config.compress ? "yes" : "no") << "\n";
  
  // TODO: Implementation
  // 1. Connect to snapshot server
  // 2. Receive snapshots
  // 3. Write to disk with rotation
  // 4. Optionally compress
  
  std::cout << "Collector stub - implementation pending\n";
  
  return 0;
}

