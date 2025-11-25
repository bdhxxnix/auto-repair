#include "domain/maintenance_detector.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <optional>
#include <sstream>

namespace {
std::optional<Part> findPart(const std::vector<Part>& stock, const std::string& id) {
  auto it = std::find_if(stock.begin(), stock.end(), [&](const Part& p){ return p.id == id; });
  if (it == stock.end()) return std::nullopt;
  return *it;
}

WOItem makeItem(const std::string& id, const std::string& name, double laborHours, double basePrice,
                const std::vector<std::pair<Part, int>>& parts, bool detected = true) {
  WOItem item;
  item.item = ServiceItem{id, name, laborHours, basePrice};
  item.parts = parts;
  item.laborHoursOverride = laborHours;
  item.autoDetected = detected;
  return item;
}
}

DetectionResult MaintenanceDetector::detect(const Vehicle& vehicle, const std::vector<Part>& stock) {
  DetectionResult result;
  auto now = std::chrono::system_clock::now();
  std::time_t tt = std::chrono::system_clock::to_time_t(now);
  std::tm local{};
#ifdef _WIN32
  localtime_s(&local, &tt);
#else
  local = *std::localtime(&tt);
#endif
  const int currentYear = 1900 + local.tm_year;
  const int age = currentYear - vehicle.year;

  auto oil = findPart(stock, "P001");
  auto oilFilter = findPart(stock, "P002");
  auto airFilter = findPart(stock, "P003");
  auto brakePads = findPart(stock, "P004");

  std::vector<std::string> notes;

  if (oil && oilFilter) {
    result.items.push_back(makeItem("S-OIL", "Oil & Filter Change", 0.6, 35,
                                     {{*oil, 4}, {*oilFilter, 1}}));
    notes.push_back("Added oil and filter change");
  }

  if (age >= 3 && airFilter) {
    result.items.push_back(makeItem("S-AIR", "Cabin Air Filter", 0.3, 20,
                                     {{*airFilter, 1}}));
    notes.push_back("Refreshed cabin air filter for vehicles older than 3 years");
  }

  if (age >= 5 && brakePads) {
    result.items.push_back(makeItem("S-BRAKE", "Brake Pad Inspection", 0.8, 45,
                                     {{*brakePads, 1}}));
    notes.push_back("Included brake pad check for 5+ year old vehicles");
  }

  if (result.items.empty()) {
    // Fallback when no parts are recognized
    result.items.push_back(makeItem("S-CHECK", "General Inspection", 0.5, 30, {}, true));
    notes.push_back("Fallback general inspection used (no matching parts found)");
  }

  std::ostringstream oss;
  for (size_t i = 0; i < notes.size(); ++i) {
    if (i) oss << "; ";
    oss << notes[i];
  }
  result.note = oss.str();
  return result;
}

