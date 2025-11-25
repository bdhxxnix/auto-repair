
#include "domain/work_order.hpp"
#include <stdexcept>

WorkOrder::WorkOrder(const WorkOrder& other)
  : id(other.id), vehicle(other.vehicle), advisor(other.advisor), tech(other.tech), customer(other.customer),
    items(other.items), status(other.status), detectionNote(other.detectionNote),
    pricing(other.pricing ? other.pricing->clone() : std::make_unique<NormalPricing>()) {}

WorkOrder& WorkOrder::operator=(const WorkOrder& other) {
  if (this == &other) return *this;
  id = other.id;
  vehicle = other.vehicle;
  advisor = other.advisor;
  tech = other.tech;
  customer = other.customer;
  items = other.items;
  status = other.status;
  detectionNote = other.detectionNote;
  pricing = other.pricing ? other.pricing->clone() : std::make_unique<NormalPricing>();
  return *this;
}

static double compute_sum(const std::vector<WOItem>& items, double techHourly) {
  double total = 0;
  for (const auto& it : items) {
    double h = (it.laborHoursOverride > 0 ? it.laborHoursOverride : it.item.laborHours);
    total += it.item.basePrice + h * techHourly;
    for (const auto& pr : it.parts) total += pr.first.unitPrice * pr.second;
  }
  return total;
}

double NormalPricing::calcTotal(const std::vector<WOItem>& items, double techHourly) const {
  return compute_sum(items, techHourly);
}
double MemberDiscountPricing::calcTotal(const std::vector<WOItem>& items, double techHourly) const {
  return compute_sum(items, techHourly) * rate_;
}
double CampaignPricing::calcTotal(const std::vector<WOItem>& items, double techHourly) const {
  return compute_sum(items, techHourly) * rate_;
}

void WorkOrder::assign(const Technician& t) {
  if (status != WOStatus::Draft) throw std::runtime_error("Can only assign in Draft");
  tech = t;
  if (!id.empty()) tech.assignedWorkOrders.push_back(id);
  status = WOStatus::Assigned;
}
void WorkOrder::start() {
  if (status != WOStatus::Assigned) throw std::runtime_error("Can only start after Assigned");
  status = WOStatus::InProgress;
}
void WorkOrder::complete() {
  if (status != WOStatus::InProgress) throw std::runtime_error("Can only complete after InProgress");
  status = WOStatus::Completed;
}
double WorkOrder::previewTotal() const {
  double techHourly = tech.hourlyRate;
  return pricing->calcTotal(items, techHourly);
}
double WorkOrder::settle() {
  if (status != WOStatus::Completed) throw std::runtime_error("Cannot settle before Completed");
  double total = previewTotal();
  status = WOStatus::Paid;
  return total;
}
