
#pragma once
#include <vector>
#include <optional>
#include <string>

template <typename T>
class Repository {
public:
  virtual ~Repository() = default;
  virtual void save(const T& t) = 0;
  virtual std::optional<T> findById(const std::string& id) = 0;
  virtual std::vector<T> findAll() = 0;
};
