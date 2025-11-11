
#pragma once
#include <string>
struct Person {
  std::string id;
  std::string name;
  virtual ~Person() = default;
};
