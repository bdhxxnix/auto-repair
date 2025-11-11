
#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>
#include "domain/part.hpp"
#include "observer.hpp"

class Inventory {
  std::unordered_map<std::string, Part> parts_;
  std::vector<InventoryObserver*> obs_;
public:
  void addObserver(InventoryObserver* o) { obs_.push_back(o); }
  void upsert(const Part& p) { parts_[p.id] = p; }
  bool has(const std::string& id) const { return parts_.count(id) > 0; }
  Part get(const std::string& id) const {
    auto it = parts_.find(id);
    if (it == parts_.end()) throw std::runtime_error("part not found");
    return it->second;
  }
  bool consume(const std::string& id, int qty) {
    auto it = parts_.find(id);
    if (it == parts_.end() || it->second.stock < qty) return false;
    it->second.stock -= qty;
    if (it->second.stock <= it->second.reorderPoint) {
      for (auto* o : obs_) o->onLowStock(id, it->second.stock);
    }
    return true;
  }
};
