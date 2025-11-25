
#include <QApplication>
#include "ui/gui.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  // Seed demo data
  DataStore store;
  Customer c1{"C001", "Alice", "1380000", 1};
  Customer c2{"C002", "Bob", "1370000", 0};
  store.customers.push_back(c1);
  store.customers.push_back(c2);

  Vehicle v1{"VIN123", "渝A88888", "Toyota", "Corolla", 2020, c1.id};
  Vehicle v2{"VIN456", "渝B66666", "Honda", "Civic", 2021, c2.id};
  store.vehicles.push_back(v1);
  store.vehicles.push_back(v2);

  Technician t1; t1.id = "E100"; t1.name = "Bob"; t1.hourlyRate = 120; t1.hoursWorked = 8;
  Technician t2; t2.id = "E101"; t2.name = "Charlie"; t2.hourlyRate = 150; t2.hoursWorked = 6;
  store.technicians.push_back(t1);
  store.technicians.push_back(t2);

  WorkOrder wo;
  wo.id = "WO0001";
  wo.vehicle = v1;
  wo.customer = c1;
  wo.advisor.id = "E200";
  wo.advisor.name = "Eve";
  WOItem item;
  item.item = ServiceItem{"S001", "更换机油", 0.5, 20};
  item.laborHoursOverride = 0.5;
  wo.items.push_back(item);
  wo.assign(t1);
  wo.start();
  wo.complete();
  wo.settle();
  store.workOrders.push_back(wo);

  MainWindow w(store);
  w.show();
  return app.exec();
}
