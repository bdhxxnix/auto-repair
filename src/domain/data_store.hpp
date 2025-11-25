#pragma once

#include <vector>
#include "customer.hpp"
#include "vehicle.hpp"
#include "employee.hpp"
#include "work_order.hpp"
#include "part.hpp"

struct DataStore {
  std::vector<Customer> customers;
  std::vector<Vehicle> vehicles;
  std::vector<Technician> technicians;
  std::vector<WorkOrder> workOrders;
  std::vector<Part> parts;
};

