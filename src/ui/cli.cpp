
#include "ui/cli.hpp"
#include <iostream>
#include <utility>
#include "persistence/file_repository.hpp"
#include "report/report_service.hpp"

void ReorderNotifier::onLowStock(const std::string& partId, int stock) {
  std::cout << "[ALERT] Part " << partId << " low stock: " << stock << std::endl;
}

void CLI::demo() {
  // Setup repositories & inventory
  PartCsvRepository repo("data/parts.csv");
  Inventory inv;
  ReorderNotifier notifier;
  inv.addObserver(&notifier);

  // Seed parts (will persist)
  Part oil{"P001","Engine Oil",50,5,3};
  Part filter{"P002","Oil Filter",30,2,2};
  repo.save(oil); repo.save(filter);
  inv.upsert(oil); inv.upsert(filter);

  // Domain objects
  Customer c{"C001","Alice","1380000",1};
  Vehicle v{"VIN123","渝A88888","Toyota","Corolla",2020,c.id};
  Technician t; t.id="E100"; t.name="Bob"; t.hourlyRate=120; t.hoursWorked=8;
  ServiceAdvisor sa; sa.id="E200"; sa.name="Eve"; sa.baseSalary=6000; sa.commission=500;

  // Build a work order
  WorkOrder wo;
  wo.id="WO0001";
  wo.vehicle=v; wo.advisor=sa; wo.tech=t; wo.customer=c;

  WOItem i1;
  i1.item = ServiceItem{"S001","更换机油",0.5,20};
  i1.parts.push_back({repo.findById("P001").value(),1}); // oil
  i1.parts.push_back({repo.findById("P002").value(),1}); // filter
  wo.items.push_back(i1);

  // Member discount for VIP
  if (c.level == 1) {
    wo.pricing = std::make_unique<MemberDiscountPricing>(0.9);
  }

  // Flow: assign → start → complete → settle
  wo.assign(t);
  wo.start();
  wo.complete();
  double total = wo.settle();

  // Inventory consume
  inv.consume("P001",1);
  inv.consume("P002",1);

  // Show result
  std::cout << "WorkOrder " << wo.id << " settled. Total: " << total << std::endl;
  // Simple report
  std::vector<WorkOrder> orders;
  orders.push_back(std::move(wo));
  auto rep = ReportService::turnover(orders);
  std::cout << "Paid Orders: " << rep.count << ", Turnover: " << rep.total << std::endl;
  std::cout << "Paid Orders: " << rep.count << ", Turnover: " << rep.total << std::endl;
}
