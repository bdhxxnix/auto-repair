#pragma once

#include <string>
#include <vector>
#include "vehicle.hpp"
#include "work_order.hpp"

struct DetectionResult {
  std::vector<WOItem> items;
  std::string note;
};

// A light-weight ruleset that inspects the vehicle and selects
// maintenance packages + required parts.
class MaintenanceDetector {
public:
  static DetectionResult detect(const Vehicle& vehicle, const std::vector<Part>& stock);
};

