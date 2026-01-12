#ifndef HFT_ELEMENTS_METRICS_REGISTRY_H
#define HFT_ELEMENTS_METRICS_REGISTRY_H

#include "hft_elements/metrics/histogram.h"
#include "hft_elements/metrics/counter.h"
#include "hft_elements/metrics/gauge.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace hft_elements {
namespace metrics {

// Central registry for all metrics
class MetricsRegistry {
public:
  MetricsRegistry() noexcept = default;
  
  // Get or create histogram
  Histogram* get_histogram(const std::string& name);
  
  // Get or create counter
  Counter* get_counter(const std::string& name);
  
  // Get or create gauge
  Gauge* get_gauge(const std::string& name);
  
  // Export all metrics to Prometheus format
  std::string export_prometheus() const;
  
  // Reset all metrics
  void reset_all() noexcept;

private:
  std::unordered_map<std::string, std::unique_ptr<Histogram>> histograms_;
  std::unordered_map<std::string, std::unique_ptr<Counter>> counters_;
  std::unordered_map<std::string, std::unique_ptr<Gauge>> gauges_;
};

// Global registry instance
MetricsRegistry& global_registry();

} // namespace metrics
} // namespace hft_elements

#endif


