
#include <QApplication>
#include <algorithm>
#include "ui/gui.hpp"
#include "persistence/data_store_persistence.hpp"
#include "persistence/file_repository.hpp"
#include "domain/maintenance_detector.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  const std::string dataPath = "data/data_store.json";
  DataStore store = DataStorePersistence::load(dataPath);
  if (store.customers.empty() && store.vehicles.empty() && store.workOrders.empty()) {
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

    PartCsvRepository partRepo("data/parts.csv");
    auto parts = partRepo.findAll();
    if (parts.empty()) {
      Part oil{"P001","Engine Oil",50,30,5,100};
      Part filter{"P002","Oil Filter",30,25,4,80};
      Part air{"P003","Air Filter",45,18,3,60};
      Part brake{"P004","Brake Pads",120,10,2,40};
      partRepo.save(oil);
      partRepo.save(filter);
      partRepo.save(air);
      partRepo.save(brake);
      parts = partRepo.findAll();
    }
    store.parts = parts;

    WorkOrder wo;
    wo.id = "WO0001";
    wo.vehicle = v1;
    wo.customer = c1;
    wo.advisor.id = "E200";
    wo.advisor.name = "Eve";
    auto detection = MaintenanceDetector::detect(v1, store.parts);
    wo.items = detection.items;
    wo.detectionNote = detection.note;
    wo.assign(t1);
    auto techIt = std::find_if(store.technicians.begin(), store.technicians.end(), [&](const Technician& t){ return t.id == t1.id; });
    if (techIt != store.technicians.end()) techIt->assignedWorkOrders.push_back(wo.id);
    wo.start();
    wo.complete();
    wo.settle();
    store.workOrders.push_back(wo);
    DataStorePersistence::save(store, dataPath);
  } else if (store.parts.empty()) {
    PartCsvRepository partRepo("data/parts.csv");
    store.parts = partRepo.findAll();
  }

  MainWindow w(store, QString::fromStdString(dataPath));
  w.show();
  return app.exec();
}
