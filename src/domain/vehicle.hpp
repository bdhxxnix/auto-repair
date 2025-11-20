
#pragma once
#include <string>
#include "customer.hpp"

struct Vehicle {
  std::string vin;
  std::string plate;
  std::string brand;
  std::string model;
  int year{0};
  std::string ownerId;
};
