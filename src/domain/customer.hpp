
#pragma once
#include <string>
#include <vector>

struct Customer {
  std::string id;
  std::string name;
  std::string phone;
  int level{0}; // 0: normal, 1: VIP
};
