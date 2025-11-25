
#include "ui/cli.hpp"
#include <iostream>
#include <utility>
#include "persistence/file_repository.hpp"
#include "report/report_service.hpp"
#include "domain/maintenance_detector.hpp"
#include "inventory/store_house.hpp"

void ReorderNotifier::onLowStock(const std::string& partId, int stock) {
  std::cout << "[ALERT] Part " << partId << " low stock: " << stock << std::endl;
}

void CLI::demo() {
  // Setup repositories & inventory
  PartCsvRepository repo("data/parts.csv");
  StoreHouse store(repo.findAll());
  ReorderNotifier notifier;
  store.addObserver(&notifier);

  // Seed parts (will persist)
  if (store.snapshot().empty()) {
    Part oil{"P001","Engine Oil",50,30,5,100};
    Part filter{"P002","Oil Filter",30,25,4,80};
    Part air{"P003","Air Filter",45,18,3,60};
    Part brake{"P004","Brake Pads",120,10,2,40};
    repo.save(oil); repo.save(filter); repo.save(air); repo.save(brake);
    store.seed(repo.findAll());
  }

  // Domain objects
  Customer c{"C001","Alice","1380000",1};
  Vehicle v{"VIN123","渝A88888","Toyota","Corolla",2020,c.id};
  Technician t; t.id="E100"; t.name="Bob"; t.hourlyRate=120; t.hoursWorked=8;
  ServiceAdvisor sa; sa.id="E200"; sa.name="Eve"; sa.baseSalary=6000; sa.commission=500;

  // Build a work order
  WorkOrder wo;
  wo.id="WO0001";
  wo.vehicle=v; wo.advisor=sa; wo.tech=t; wo.customer=c;

  auto detection = MaintenanceDetector::detect(v, store.snapshot());
  wo.items = detection.items;
  wo.detectionNote = detection.note;

  // Member discount for VIP
  if (c.level == 1) {
    wo.pricing = std::make_unique<MemberDiscountPricing>(0.9);
  }

  // Flow: assign → start → complete → settle
  wo.assign(t);
  wo.start();
  wo.complete();
  double total = wo.settle();

  // Inventory consume + alerting
  if (!store.consumeForOrder(wo)) {
    std::cout << "[WARN] Not enough stock for all detected parts" << std::endl;
  }
  for (const auto& alert : store.takeAlerts()) {
    std::cout << "[ALERT] " << alert << std::endl;
  }

  // Show result
  std::cout << "WorkOrder " << wo.id << " settled. Total: " << total << std::endl;
  // Simple report
  std::vector<WorkOrder> orders;
  orders.push_back(std::move(wo));
  auto rep = ReportService::turnover(orders);
  std::cout << "Paid Orders: " << rep.count << ", Turnover: " << rep.total << std::endl;
  std::cout << "Paid Orders: " << rep.count << ", Turnover: " << rep.total << std::endl;
}
