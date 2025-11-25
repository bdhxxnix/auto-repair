#include "inventory/store_house.hpp"

#include <algorithm>
#include <sstream>

StoreHouse::StoreHouse() {
  inventory_.addObserver(this);
}

StoreHouse::StoreHouse(const std::vector<Part>& seedParts) : StoreHouse() {
  seed(seedParts);
}

void StoreHouse::seed(const std::vector<Part>& parts) {
  for (const auto& p : parts) {
    inventory_.upsert(p);
  }
}

std::vector<Part> StoreHouse::snapshot() const {
  return inventory_.list();
}

bool StoreHouse::consume(const std::string& partId, int qty) {
  return inventory_.consume(partId, qty);
}

bool StoreHouse::consumeForOrder(const WorkOrder& order) {
  bool ok = true;
  // Pre-flight check to ensure all parts exist and have enough stock
  for (const auto& item : order.items) {
    for (const auto& pair : item.parts) {
      auto existing = inventory_.find(pair.first.id);
      if (!existing || existing->stock < pair.second) {
        ok = false;
      }
    }
  }
  if (!ok) return false;

  for (const auto& item : order.items) {
    for (const auto& pair : item.parts) {
      if (!consume(pair.first.id, pair.second)) {
        ok = false;
      }
    }
  }
  return ok;
}

std::vector<std::string> StoreHouse::takeAlerts() {
  auto copy = alerts_;
  alerts_.clear();
  return copy;
}

void StoreHouse::onLowStock(const std::string& partId, int stock) {
  std::ostringstream oss;
  oss << "Part " << partId << " low stock: " << stock;
  alerts_.push_back(oss.str());
}

