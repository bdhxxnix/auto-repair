#pragma once

#include <string>
#include <vector>
#include "inventory.hpp"
#include "domain/work_order.hpp"

// Centralized inventory house that keeps track of parts, consumption,
// and low-stock alerts.
class StoreHouse : public InventoryObserver {
public:
  StoreHouse();
  explicit StoreHouse(const std::vector<Part>& seedParts);

  void seed(const std::vector<Part>& parts);
  void addObserver(InventoryObserver* obs) { inventory_.addObserver(obs); }
  std::vector<Part> snapshot() const;
  bool consume(const std::string& partId, int qty);
  bool consumeForOrder(const WorkOrder& order);
  std::vector<std::string> takeAlerts();

  void onLowStock(const std::string& partId, int stock) override;

private:
  Inventory inventory_;
  std::vector<std::string> alerts_;
};

