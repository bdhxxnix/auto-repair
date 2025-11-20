
#pragma once
#include <vector>
#include "domain/work_order.hpp"
#include "inventory/inventory.hpp"

class ReorderNotifier : public InventoryObserver {
public:
  void onLowStock(const std::string& partId, int stock) override;
};

class CLI {
public:
  static void demo();
};
