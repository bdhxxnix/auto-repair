#include "persistence/data_store_persistence.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <algorithm>
#include <memory>

namespace {
QString statusToString(WOStatus st) {
  switch (st) {
    case WOStatus::Draft: return "Draft";
    case WOStatus::Assigned: return "Assigned";
    case WOStatus::InProgress: return "InProgress";
    case WOStatus::Completed: return "Completed";
    case WOStatus::Paid: return "Paid";
    case WOStatus::Cancelled: return "Cancelled";
  }
  return "Draft";
}

WOStatus statusFromString(const QString& str) {
  if (str == "Assigned") return WOStatus::Assigned;
  if (str == "InProgress") return WOStatus::InProgress;
  if (str == "Completed") return WOStatus::Completed;
  if (str == "Paid") return WOStatus::Paid;
  if (str == "Cancelled") return WOStatus::Cancelled;
  return WOStatus::Draft;
}

QJsonObject customerToJson(const Customer& c) {
  return QJsonObject{{"id", QString::fromStdString(c.id)}, {"name", QString::fromStdString(c.name)},
                     {"phone", QString::fromStdString(c.phone)}, {"level", c.level}};
}

Customer customerFromJson(const QJsonObject& o) {
  Customer c;
  c.id = o.value("id").toString().toStdString();
  c.name = o.value("name").toString().toStdString();
  c.phone = o.value("phone").toString().toStdString();
  c.level = o.value("level").toInt();
  return c;
}

QJsonObject vehicleToJson(const Vehicle& v) {
  return QJsonObject{{"vin", QString::fromStdString(v.vin)}, {"plate", QString::fromStdString(v.plate)},
                     {"brand", QString::fromStdString(v.brand)}, {"model", QString::fromStdString(v.model)},
                     {"year", v.year}, {"ownerId", QString::fromStdString(v.ownerId)}};
}

Vehicle vehicleFromJson(const QJsonObject& o) {
  Vehicle v;
  v.vin = o.value("vin").toString().toStdString();
  v.plate = o.value("plate").toString().toStdString();
  v.brand = o.value("brand").toString().toStdString();
  v.model = o.value("model").toString().toStdString();
  v.year = o.value("year").toInt();
  v.ownerId = o.value("ownerId").toString().toStdString();
  return v;
}

QJsonObject technicianToJson(const Technician& t) {
  QJsonArray assigned;
  for (const auto& id : t.assignedWorkOrders) assigned.push_back(QString::fromStdString(id));
  return QJsonObject{{"id", QString::fromStdString(t.id)}, {"name", QString::fromStdString(t.name)},
                     {"hourlyRate", t.hourlyRate}, {"hoursWorked", t.hoursWorked},
                     {"assigned", assigned}};
}

QJsonObject advisorToJson(const ServiceAdvisor& a) {
  return QJsonObject{{"id", QString::fromStdString(a.id)}, {"name", QString::fromStdString(a.name)},
                     {"baseSalary", a.baseSalary}, {"commission", a.commission}};
}

Technician technicianFromJson(const QJsonObject& o) {
  Technician t;
  t.id = o.value("id").toString().toStdString();
  t.name = o.value("name").toString().toStdString();
  t.hourlyRate = o.value("hourlyRate").toDouble();
  t.hoursWorked = o.value("hoursWorked").toInt();
  auto assigned = o.value("assigned").toArray();
  for (const auto& v : assigned) t.assignedWorkOrders.push_back(v.toString().toStdString());
  return t;
}

ServiceAdvisor advisorFromJson(const QJsonObject& o) {
  ServiceAdvisor a;
  a.id = o.value("id").toString().toStdString();
  a.name = o.value("name").toString().toStdString();
  a.baseSalary = o.value("baseSalary").toDouble();
  a.commission = o.value("commission").toDouble();
  return a;
}

QJsonObject partToJson(const Part& p) {
  return QJsonObject{{"id", QString::fromStdString(p.id)}, {"name", QString::fromStdString(p.name)},
                     {"unitPrice", p.unitPrice}, {"stock", p.stock}, {"reorderPoint", p.reorderPoint},
                     {"capacity", p.capacity}};
}

Part partFromJson(const QJsonObject& o) {
  Part p;
  p.id = o.value("id").toString().toStdString();
  p.name = o.value("name").toString().toStdString();
  p.unitPrice = o.value("unitPrice").toDouble();
  p.stock = o.value("stock").toInt();
  p.reorderPoint = o.value("reorderPoint").toInt();
  p.capacity = o.value("capacity").toInt();
  return p;
}

QJsonObject serviceToJson(const ServiceItem& s) {
  return QJsonObject{{"id", QString::fromStdString(s.id)}, {"name", QString::fromStdString(s.name)},
                     {"laborHours", s.laborHours}, {"basePrice", s.basePrice}};
}

ServiceItem serviceFromJson(const QJsonObject& o) {
  ServiceItem s;
  s.id = o.value("id").toString().toStdString();
  s.name = o.value("name").toString().toStdString();
  s.laborHours = o.value("laborHours").toDouble();
  s.basePrice = o.value("basePrice").toDouble();
  return s;
}

QString pricingToString(const PricingStrategy& p) {
  if (dynamic_cast<const MemberDiscountPricing*>(&p)) return "Member";
  if (dynamic_cast<const CampaignPricing*>(&p)) return "Campaign";
  return "Normal";
}

std::unique_ptr<PricingStrategy> pricingFromString(const QString& p) {
  if (p == "Member") return std::make_unique<MemberDiscountPricing>();
  if (p == "Campaign") return std::make_unique<CampaignPricing>();
  return std::make_unique<NormalPricing>();
}

QJsonObject woItemToJson(const WOItem& item) {
  QJsonObject obj;
  obj.insert("service", serviceToJson(item.item));
  obj.insert("laborOverride", item.laborHoursOverride);
  obj.insert("autoDetected", item.autoDetected);
  QJsonArray parts;
  for (const auto& pr : item.parts) {
    QJsonObject pairObj;
    pairObj.insert("part", partToJson(pr.first));
    pairObj.insert("qty", pr.second);
    parts.push_back(pairObj);
  }
  obj.insert("parts", parts);
  return obj;
}

WOItem woItemFromJson(const QJsonObject& o) {
  WOItem it;
  it.item = serviceFromJson(o.value("service").toObject());
  it.laborHoursOverride = o.value("laborOverride").toDouble(-1);
  it.autoDetected = o.value("autoDetected").toBool(false);
  auto partsArr = o.value("parts").toArray();
  for (const auto& v : partsArr) {
    auto pairObj = v.toObject();
    it.parts.push_back({partFromJson(pairObj.value("part").toObject()), pairObj.value("qty").toInt()});
  }
  return it;
}

QJsonObject workOrderToJson(const WorkOrder& w) {
  QJsonObject obj;
  obj.insert("id", QString::fromStdString(w.id));
  obj.insert("vehicle", vehicleToJson(w.vehicle));
  obj.insert("advisor", advisorToJson(w.advisor));
  obj.insert("tech", technicianToJson(w.tech));
  obj.insert("customer", customerToJson(w.customer));
  obj.insert("status", statusToString(w.status));
  obj.insert("detectionNote", QString::fromStdString(w.detectionNote));
  obj.insert("pricing", pricingToString(*w.pricing));
  QJsonArray items;
  for (const auto& it : w.items) items.push_back(woItemToJson(it));
  obj.insert("items", items);
  return obj;
}

WorkOrder workOrderFromJson(const QJsonObject& o) {
  WorkOrder w;
  w.id = o.value("id").toString().toStdString();
  w.vehicle = vehicleFromJson(o.value("vehicle").toObject());
  w.advisor = advisorFromJson(o.value("advisor").toObject());
  w.tech = technicianFromJson(o.value("tech").toObject());
  w.customer = customerFromJson(o.value("customer").toObject());
  w.status = statusFromString(o.value("status").toString());
  w.detectionNote = o.value("detectionNote").toString().toStdString();
  w.pricing = pricingFromString(o.value("pricing").toString());
  auto itemsArr = o.value("items").toArray();
  for (const auto& v : itemsArr) w.items.push_back(woItemFromJson(v.toObject()));
  return w;
}
}

DataStore DataStorePersistence::load(const std::string& path) {
  QFile file(QString::fromStdString(path));
  DataStore store;
  if (!file.exists()) return store;
  if (!file.open(QIODevice::ReadOnly)) return store;
  auto doc = QJsonDocument::fromJson(file.readAll());
  if (!doc.isObject()) return store;
  auto root = doc.object();

  for (const auto& v : root.value("customers").toArray()) store.customers.push_back(customerFromJson(v.toObject()));
  for (const auto& v : root.value("vehicles").toArray()) store.vehicles.push_back(vehicleFromJson(v.toObject()));
  for (const auto& v : root.value("technicians").toArray()) store.technicians.push_back(technicianFromJson(v.toObject()));
  for (const auto& v : root.value("parts").toArray()) store.parts.push_back(partFromJson(v.toObject()));
  for (const auto& v : root.value("workOrders").toArray()) store.workOrders.push_back(workOrderFromJson(v.toObject()));

  // Refresh assigned work orders from orders to ensure consistency
  for (auto& t : store.technicians) t.assignedWorkOrders.clear();
  for (const auto& w : store.workOrders) {
    auto it = std::find_if(store.technicians.begin(), store.technicians.end(), [&](const Technician& t){ return t.id == w.tech.id; });
    if (it != store.technicians.end()) it->assignedWorkOrders.push_back(w.id);
  }
  return store;
}

void DataStorePersistence::save(const DataStore& store, const std::string& path) {
  QJsonObject root;
  QJsonArray customers;
  for (const auto& c : store.customers) customers.push_back(customerToJson(c));
  root.insert("customers", customers);

  QJsonArray vehicles;
  for (const auto& v : store.vehicles) vehicles.push_back(vehicleToJson(v));
  root.insert("vehicles", vehicles);

  QJsonArray technicians;
  for (const auto& t : store.technicians) technicians.push_back(technicianToJson(t));
  root.insert("technicians", technicians);

  QJsonArray parts;
  for (const auto& p : store.parts) parts.push_back(partToJson(p));
  root.insert("parts", parts);

  QJsonArray workOrders;
  for (const auto& w : store.workOrders) workOrders.push_back(workOrderToJson(w));
  root.insert("workOrders", workOrders);

  QJsonDocument doc(root);
  QSaveFile file(QString::fromStdString(path));
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
  file.write(doc.toJson());
  file.commit();
}

