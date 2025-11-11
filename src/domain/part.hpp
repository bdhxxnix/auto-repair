
#pragma once
#include <string>

struct Part {
  std::string id;
  std::string name;
  double unitPrice{0};
  int stock{0};
  int reorderPoint{3};
};
