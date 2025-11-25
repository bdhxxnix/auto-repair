#pragma once

#include <string>
#include "domain/data_store.hpp"

class DataStorePersistence {
public:
  static DataStore load(const std::string& path);
  static void save(const DataStore& store, const std::string& path);
};

