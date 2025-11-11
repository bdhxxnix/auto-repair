
#pragma once
#include <vector>
#include <string>
#include <memory>
#include "customer.hpp"
#include "vehicle.hpp"
#include "employee.hpp"
#include "part.hpp"
#include "service_item.hpp"

enum class WOStatus { Draft, Assigned, InProgress, Completed, Paid, Cancelled };

struct WOItem {
  ServiceItem item;
  std::vector<std::pair<Part, int>> parts; // (part, qty)
  double laborHoursOverride{-1};
};

class PricingStrategy {
public:
  virtual ~PricingStrategy() = default;
  virtual double calcTotal(const std::vector<WOItem>& items, double techHourly) const = 0;
};

class NormalPricing final : public PricingStrategy {
public:
  double calcTotal(const std::vector<WOItem>& items, double techHourly) const override;
};
class MemberDiscountPricing final : public PricingStrategy {
  double rate_{0.9};
public:
  explicit MemberDiscountPricing(double rate=0.9): rate_(rate) {}
  double calcTotal(const std::vector<WOItem>& items, double techHourly) const override;
};
class CampaignPricing final : public PricingStrategy {
  double rate_{0.8};
public:
  explicit CampaignPricing(double rate=0.8): rate_(rate) {}
  double calcTotal(const std::vector<WOItem>& items, double techHourly) const override;
};

class WorkOrder {
public:
  std::string id;
  Vehicle vehicle;
  ServiceAdvisor advisor;
  Technician tech;
  Customer customer;

  std::vector<WOItem> items;
  WOStatus status{WOStatus::Draft};
  std::unique_ptr<PricingStrategy> pricing{std::make_unique<NormalPricing>()};

  void assign(const Technician& t);
  void start();
  void complete();
  double settle(); // sets status to Paid and returns total
  double previewTotal() const; // helper
};
