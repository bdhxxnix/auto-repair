
#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>
#include <cmath>
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
  std::optional<Part> find(const std::string& id) const {
    auto it = parts_.find(id);
    if (it == parts_.end()) return std::nullopt;
    return it->second;
  }
  std::vector<Part> list() const {
    std::vector<Part> out;
    out.reserve(parts_.size());
    for (const auto& kv : parts_) out.push_back(kv.second);
    return out;
  }
  bool consume(const std::string& id, int qty) {
    auto it = parts_.find(id);
    if (it == parts_.end() || it->second.stock < qty) return false;
    it->second.stock -= qty;
    bool lowByReorder = it->second.stock <= it->second.reorderPoint;
    bool lowByCapacity = false;
    if (it->second.capacity > 0) {
      auto threshold = static_cast<int>(std::ceil(it->second.capacity * 0.1));
      lowByCapacity = it->second.stock <= threshold;
    }
    if (lowByReorder || lowByCapacity) {
      for (auto* o : obs_) o->onLowStock(id, it->second.stock);
    }
    return true;
  }
};
