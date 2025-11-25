
#pragma once
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "repository.hpp"
#include "domain/part.hpp"

// A tiny CSV repository for Part as a demo
class PartCsvRepository : public Repository<Part> {
  std::string path_;
  std::unordered_map<std::string, Part> cache_;
  void load() {
    cache_.clear();
    std::ifstream in(path_);
    if (!in.good()) return;
    std::string line;
    while (std::getline(in, line)) {
      if (line.empty()) continue;
      std::stringstream ss(line);
      Part p; char comma;
      std::getline(ss, p.id, ',');
      std::getline(ss, p.name, ',');
      ss >> p.unitPrice >> comma >> p.stock >> comma >> p.reorderPoint >> comma >> p.capacity;
      cache_[p.id] = p;
    }
  }
  void flush() {
    std::ofstream out(path_, std::ios::trunc);
    for (auto& kv : cache_) {
      const auto& p = kv.second;
      out << p.id << "," << p.name << "," << p.unitPrice << "," << p.stock << "," << p.reorderPoint << "," << p.capacity << "\n";
    }
  }
public:
  explicit PartCsvRepository(std::string path) : path_(std::move(path)) { load(); }
  void save(const Part& t) override { cache_[t.id] = t; flush(); }
  std::optional<Part> findById(const std::string& id) override {
    auto it = cache_.find(id);
    if (it == cache_.end()) return std::nullopt;
    return it->second;
  }
  std::vector<Part> findAll() override {
    std::vector<Part> v; v.reserve(cache_.size());
    for (auto& kv : cache_) v.push_back(kv.second);
    return v;
  }
};
