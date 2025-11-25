
#pragma once
#include <vector>
#include <string>
#include <map>
#include "domain/work_order.hpp"
#include "domain/part.hpp"

struct TurnoverReport {
  double total{0};
  int count{0};
};

struct FinalReport {
  double turnover{0};
  int paidCount{0};
  std::map<WOStatus, std::vector<std::string>> statusBuckets;
  std::vector<Part> inventory;
};

class ReportService {
public:
  static TurnoverReport turnover(const std::vector<WorkOrder>& orders) {
    TurnoverReport r;
    for (const auto& o : orders) {
      if (o.status == WOStatus::Paid) {
        r.total += o.previewTotal();
        r.count++;
      }
    }
    return r;
  }

  static FinalReport summary(const std::vector<WorkOrder>& orders, const std::vector<Part>& inventory) {
    FinalReport r;
    r.inventory = inventory;
    for (const auto& o : orders) {
      r.statusBuckets[o.status].push_back(o.id);
      if (o.status == WOStatus::Paid) {
        r.paidCount++;
        r.turnover += o.previewTotal();
      }
    }
    return r;
  }
};
