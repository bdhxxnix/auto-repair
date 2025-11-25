
#pragma once
#include <vector>
#include <string>
#include "person.hpp"

struct Employee : public Person {
  virtual double calculatePay() const = 0;
  virtual ~Employee() = default;
};

struct Technician : public Employee {
  double hourlyRate{120.0};
  int hoursWorked{0};
  std::vector<std::string> assignedWorkOrders;
  double calculatePay() const override { return hourlyRate * hoursWorked; }
};

struct ServiceAdvisor : public Employee {
  double baseSalary{6000.0};
  double commission{0.0};
  double calculatePay() const override { return baseSalary + commission; }
};

struct Manager : public Employee {
  double baseSalary{10000.0};
  double bonus{0.0};
  double calculatePay() const override { return baseSalary + bonus; }
};
