
#pragma once
#include <string>
class InventoryObserver {
public:
  virtual ~InventoryObserver() = default;
  virtual void onLowStock(const std::string& partId, int stock) = 0;
};
