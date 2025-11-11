
#pragma once
#include <vector>
#include <string>
#include <map>
#include "domain/work_order.hpp"

struct TurnoverReport {
  double total{0};
  int count{0};
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
};
