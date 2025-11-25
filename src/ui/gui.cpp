#include "ui/gui.hpp"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QHeaderView>
#include <QDoubleSpinBox>
#include <algorithm>
#include <map>
#include <cstdio>
#include "report/report_service.hpp"

namespace {
std::string generateId(const std::string& prefix, int number) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%s%04d", prefix.c_str(), number);
  return std::string(buf);
}

QString statusToText(WOStatus st) {
  switch (st) {
    case WOStatus::Draft: return "Draft";
    case WOStatus::Assigned: return "Assigned";
    case WOStatus::InProgress: return "In Progress";
    case WOStatus::Completed: return "Completed";
    case WOStatus::Paid: return "Paid";
    case WOStatus::Cancelled: return "Cancelled";
  }
  return "Unknown";
}
}

MainWindow::MainWindow(DataStore store, QWidget* parent)
    : QMainWindow(parent), store_(std::move(store)) {
  setupUI();
  populateCustomers();
  populateTechnicians();
  populateWorkOrders();
  populateSelectors();
  updateSummary();
}

void MainWindow::setupUI() {
  resize(1000, 700);
  tabs_ = new QTabWidget(this);
  setCentralWidget(tabs_);
  tabs_->addTab(buildCustomerPage(), tr("Customers"));
  tabs_->addTab(buildWorkOrderPage(), tr("Work Orders"));
  tabs_->addTab(buildTechnicianPage(), tr("Mechanics"));
  tabs_->addTab(buildSummaryPage(), tr("Summary"));
}

QWidget* MainWindow::buildCustomerPage() {
  auto* page = new QWidget(this);
  auto* layout = new QHBoxLayout(page);

  // Left side: customer list and add form
  auto* left = new QVBoxLayout();
  customerList_ = new QListWidget(page);
  left->addWidget(new QLabel(tr("Customers"), page));
  left->addWidget(customerList_);

  auto* form = new QFormLayout();
  customerName_ = new QLineEdit(page);
  customerPhone_ = new QLineEdit(page);
  customerLevel_ = new QComboBox(page);
  customerLevel_->addItems({tr("Normal"), tr("VIP")});
  form->addRow(tr("Name"), customerName_);
  form->addRow(tr("Phone"), customerPhone_);
  form->addRow(tr("Level"), customerLevel_);
  auto* addBtn = new QPushButton(tr("Add Customer"), page);
  connect(addBtn, &QPushButton::clicked, this, &MainWindow::addCustomer);
  left->addLayout(form);
  left->addWidget(addBtn);

  layout->addLayout(left, 1);

  // Right side: vehicles for selected customer
  auto* right = new QVBoxLayout();
  right->addWidget(new QLabel(tr("Vehicles"), page));
  vehicleTable_ = new QTableWidget(0, 5, page);
  vehicleTable_->setHorizontalHeaderLabels({tr("VIN"), tr("Plate"), tr("Brand"), tr("Model"), tr("Year")});
  vehicleTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  right->addWidget(vehicleTable_);

  auto* vehicleForm = new QFormLayout();
  vin_ = new QLineEdit(page);
  plate_ = new QLineEdit(page);
  brand_ = new QLineEdit(page);
  model_ = new QLineEdit(page);
  year_ = new QSpinBox(page);
  year_->setRange(1980, 2100);
  vehicleForm->addRow(tr("VIN"), vin_);
  vehicleForm->addRow(tr("Plate"), plate_);
  vehicleForm->addRow(tr("Brand"), brand_);
  vehicleForm->addRow(tr("Model"), model_);
  vehicleForm->addRow(tr("Year"), year_);
  auto* addVehicleBtn = new QPushButton(tr("Add Vehicle"), page);
  connect(addVehicleBtn, &QPushButton::clicked, this, &MainWindow::addVehicle);
  right->addLayout(vehicleForm);
  right->addWidget(addVehicleBtn);

  layout->addLayout(right, 2);

  connect(customerList_, &QListWidget::currentRowChanged, this, &MainWindow::refreshVehicleList);
  return page;
}

QWidget* MainWindow::buildTechnicianPage() {
  auto* page = new QWidget(this);
  auto* layout = new QVBoxLayout(page);

  technicianTable_ = new QTableWidget(0, 3, page);
  technicianTable_->setHorizontalHeaderLabels({tr("ID"), tr("Name"), tr("Hourly Rate")});
  technicianTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  layout->addWidget(new QLabel(tr("Mechanics"), page));
  layout->addWidget(technicianTable_);

  auto* form = new QFormLayout();
  techId_ = new QLineEdit(page);
  techName_ = new QLineEdit(page);
  hourlyRate_ = new QSpinBox(page);
  hourlyRate_->setRange(50, 500);
  hourlyRate_->setValue(120);
  form->addRow(tr("ID"), techId_);
  form->addRow(tr("Name"), techName_);
  form->addRow(tr("Hourly Rate"), hourlyRate_);
  auto* addBtn = new QPushButton(tr("Add Mechanic"), page);
  connect(addBtn, &QPushButton::clicked, this, &MainWindow::addTechnician);

  layout->addLayout(form);
  layout->addWidget(addBtn);
  return page;
}

QWidget* MainWindow::buildWorkOrderPage() {
  auto* page = new QWidget(this);
  auto* layout = new QVBoxLayout(page);

  workOrderTable_ = new QTableWidget(0, 6, page);
  workOrderTable_->setHorizontalHeaderLabels({tr("ID"), tr("Vehicle"), tr("Customer"), tr("Mechanic"), tr("Status"), tr("Total")});
  workOrderTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  layout->addWidget(workOrderTable_);

  auto* form = new QFormLayout();
  vehicleSelect_ = new QComboBox(page);
  techSelect_ = new QComboBox(page);
  serviceId_ = new QLineEdit(page);
  serviceName_ = new QLineEdit(page);
  laborHours_ = new QDoubleSpinBox(page);
  laborHours_->setRange(0, 20);
  laborHours_->setDecimals(2);
  laborHours_->setSingleStep(0.5);
  basePrice_ = new QDoubleSpinBox(page);
  basePrice_->setRange(0, 5000);
  basePrice_->setDecimals(2);
  basePrice_->setSingleStep(10);
  laborOverride_ = new QDoubleSpinBox(page);
  laborOverride_->setRange(-1, 40);
  laborOverride_->setDecimals(2);
  laborOverride_->setSingleStep(0.5);
  laborOverride_->setValue(-1);

  form->addRow(tr("Vehicle"), vehicleSelect_);
  form->addRow(tr("Mechanic"), techSelect_);
  form->addRow(tr("Service ID"), serviceId_);
  form->addRow(tr("Service Name"), serviceName_);
  form->addRow(tr("Labor Hours"), laborHours_);
  form->addRow(tr("Base Price"), basePrice_);
  form->addRow(tr("Labor Override"), laborOverride_);

  auto* createBtn = new QPushButton(tr("Create Work Order"), page);
  connect(createBtn, &QPushButton::clicked, this, &MainWindow::createWorkOrder);

  auto* controls = new QHBoxLayout();
  auto* assignBtn = new QPushButton(tr("Assign"), page);
  auto* startBtn = new QPushButton(tr("Start"), page);
  auto* completeBtn = new QPushButton(tr("Complete"), page);
  auto* settleBtn = new QPushButton(tr("Settle"), page);
  connect(assignBtn, &QPushButton::clicked, this, &MainWindow::assignWorkOrder);
  connect(startBtn, &QPushButton::clicked, this, &MainWindow::startWorkOrder);
  connect(completeBtn, &QPushButton::clicked, this, &MainWindow::completeWorkOrder);
  connect(settleBtn, &QPushButton::clicked, this, &MainWindow::settleWorkOrder);
  controls->addWidget(assignBtn);
  controls->addWidget(startBtn);
  controls->addWidget(completeBtn);
  controls->addWidget(settleBtn);

  layout->addLayout(form);
  layout->addWidget(createBtn);
  layout->addLayout(controls);
  return page;
}

QWidget* MainWindow::buildSummaryPage() {
  auto* page = new QWidget(this);
  auto* layout = new QVBoxLayout(page);
  summaryLabel_ = new QLabel(tr("Summary"), page);
  layout->addWidget(summaryLabel_);
  statusTable_ = new QTableWidget(0, 2, page);
  statusTable_->setHorizontalHeaderLabels({tr("Status"), tr("Count")});
  statusTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  layout->addWidget(statusTable_);
  return page;
}

void MainWindow::populateCustomers() {
  customerList_->clear();
  for (const auto& c : store_.customers) {
    auto* item = new QListWidgetItem(QString::fromStdString(c.name + " (" + c.id + ")"));
    item->setData(Qt::UserRole, QString::fromStdString(c.id));
    customerList_->addItem(item);
  }
  if (!store_.customers.empty()) {
    customerList_->setCurrentRow(0);
    refreshVehicleList();
  }
}

void MainWindow::populateVehiclesForCustomer(const std::string& customerId) {
  vehicleTable_->setRowCount(0);
  for (const auto& v : store_.vehicles) {
    if (v.ownerId != customerId) continue;
    int row = vehicleTable_->rowCount();
    vehicleTable_->insertRow(row);
    vehicleTable_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(v.vin)));
    vehicleTable_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(v.plate)));
    vehicleTable_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(v.brand)));
    vehicleTable_->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(v.model)));
    vehicleTable_->setItem(row, 4, new QTableWidgetItem(QString::number(v.year)));
  }
}

void MainWindow::populateTechnicians() {
  technicianTable_->setRowCount(0);
  for (const auto& t : store_.technicians) {
    int row = technicianTable_->rowCount();
    technicianTable_->insertRow(row);
    technicianTable_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(t.id)));
    technicianTable_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(t.name)));
    technicianTable_->setItem(row, 2, new QTableWidgetItem(QString::number(t.hourlyRate)));
  }
}

void MainWindow::populateWorkOrders() {
  workOrderTable_->setRowCount(0);
  for (const auto& w : store_.workOrders) {
    int row = workOrderTable_->rowCount();
    workOrderTable_->insertRow(row);
    workOrderTable_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(w.id)));
    workOrderTable_->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(w.vehicle.plate)));
    workOrderTable_->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(w.customer.name)));
    workOrderTable_->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(w.tech.name)));
    workOrderTable_->setItem(row, 4, new QTableWidgetItem(statusToText(w.status)));
    workOrderTable_->setItem(row, 5, new QTableWidgetItem(QString::number(w.previewTotal())));
  }
}

void MainWindow::populateSelectors() {
  vehicleSelect_->clear();
  for (const auto& v : store_.vehicles) {
    bool alreadyHasOrder = std::any_of(store_.workOrders.begin(), store_.workOrders.end(), [&](const WorkOrder& w){
      return w.vehicle.vin == v.vin;
    });
    if (alreadyHasOrder) continue; // enforce one work order per vehicle
    QString label = QString::fromStdString(v.plate + " (" + v.vin + ")");
    vehicleSelect_->addItem(label, QString::fromStdString(v.vin));
  }

  techSelect_->clear();
  for (const auto& t : store_.technicians) {
    techSelect_->addItem(QString::fromStdString(t.name + " (" + t.id + ")"), QString::fromStdString(t.id));
  }
}

void MainWindow::updateSummary() {
  statusTable_->setRowCount(0);
  std::map<WOStatus, int> counts;
  for (const auto& w : store_.workOrders) counts[w.status]++;
  auto insertRow = [&](const QString& name, int count) {
    int row = statusTable_->rowCount();
    statusTable_->insertRow(row);
    statusTable_->setItem(row, 0, new QTableWidgetItem(name));
    statusTable_->setItem(row, 1, new QTableWidgetItem(QString::number(count)));
  };
  insertRow(tr("Draft"), counts[WOStatus::Draft]);
  insertRow(tr("Assigned"), counts[WOStatus::Assigned]);
  insertRow(tr("In Progress"), counts[WOStatus::InProgress]);
  insertRow(tr("Completed"), counts[WOStatus::Completed]);
  insertRow(tr("Paid"), counts[WOStatus::Paid]);

  auto report = ReportService::turnover(store_.workOrders);
  summaryLabel_->setText(tr("Paid Orders: %1, Turnover: %2").arg(report.count).arg(report.total));
}

void MainWindow::addCustomer() {
  Customer c;
  c.id = generateId("C", static_cast<int>(store_.customers.size() + 1));
  c.name = customerName_->text().toStdString();
  c.phone = customerPhone_->text().toStdString();
  c.level = customerLevel_->currentIndex();
  if (c.name.empty()) {
    QMessageBox::warning(this, tr("Validation"), tr("Customer name is required"));
    return;
  }
  store_.customers.push_back(c);
  populateCustomers();
  populateSelectors();
  customerName_->clear();
  customerPhone_->clear();
}

void MainWindow::addVehicle() {
  auto* current = customerList_->currentItem();
  if (!current) {
    QMessageBox::warning(this, tr("Validation"), tr("Select a customer first"));
    return;
  }
  Vehicle v;
  v.ownerId = current->data(Qt::UserRole).toString().toStdString();
  v.vin = vin_->text().toStdString();
  v.plate = plate_->text().toStdString();
  v.brand = brand_->text().toStdString();
  v.model = model_->text().toStdString();
  v.year = year_->value();
  if (v.vin.empty() || v.plate.empty()) {
    QMessageBox::warning(this, tr("Validation"), tr("VIN and Plate are required"));
    return;
  }
  store_.vehicles.push_back(v);
  refreshVehicleList();
  populateSelectors();
  vin_->clear(); plate_->clear(); brand_->clear(); model_->clear();
}

void MainWindow::addTechnician() {
  Technician t;
  t.id = techId_->text().toStdString();
  t.name = techName_->text().toStdString();
  t.hourlyRate = hourlyRate_->value();
  if (t.id.empty() || t.name.empty()) {
    QMessageBox::warning(this, tr("Validation"), tr("ID and Name required"));
    return;
  }
  store_.technicians.push_back(t);
  populateTechnicians();
  populateSelectors();
  techId_->clear();
  techName_->clear();
}

void MainWindow::createWorkOrder() {
  if (vehicleSelect_->count() == 0 || techSelect_->count() == 0) {
    QMessageBox::warning(this, tr("Validation"), tr("Vehicle and mechanic are required"));
    return;
  }
  QString vin = vehicleSelect_->currentData().toString();
  auto vehicleIt = std::find_if(store_.vehicles.begin(), store_.vehicles.end(), [&](const Vehicle& v){ return QString::fromStdString(v.vin) == vin; });
  if (vehicleIt == store_.vehicles.end()) return;

  bool used = std::any_of(store_.workOrders.begin(), store_.workOrders.end(), [&](const WorkOrder& w){ return w.vehicle.vin == vehicleIt->vin; });
  if (used) {
    QMessageBox::warning(this, tr("Validation"), tr("This vehicle already has a work order"));
    return;
  }

  QString techId = techSelect_->currentData().toString();
  auto techIt = std::find_if(store_.technicians.begin(), store_.technicians.end(), [&](const Technician& t){ return QString::fromStdString(t.id) == techId; });
  if (techIt == store_.technicians.end()) return;

  auto customerIt = std::find_if(store_.customers.begin(), store_.customers.end(), [&](const Customer& c){ return c.id == vehicleIt->ownerId; });
  if (customerIt == store_.customers.end()) {
    QMessageBox::warning(this, tr("Validation"), tr("Vehicle owner not found"));
    return;
  }

  WorkOrder w;
  w.id = generateId("WO", static_cast<int>(store_.workOrders.size() + 1));
  w.vehicle = *vehicleIt;
  w.customer = *customerIt;
  w.advisor.id = "SA001"; w.advisor.name = "Advisor"; // simple demo advisor

  ServiceItem si;
  si.id = serviceId_->text().toStdString();
  si.name = serviceName_->text().toStdString();
  si.laborHours = laborHours_->value();
  si.basePrice = basePrice_->value();

  if (!si.id.empty()) {
    WOItem item;
    item.item = si;
    item.laborHoursOverride = laborOverride_->value();
    w.items.push_back(item);
  }

  try {
    w.assign(*techIt);
  } catch (const std::exception& e) {
    QMessageBox::critical(this, tr("Error"), e.what());
    return;
  }

  store_.workOrders.push_back(w);
  populateWorkOrders();
  populateSelectors();
  updateSummary();
  serviceId_->clear();
  serviceName_->clear();
  laborHours_->setValue(0);
  basePrice_->setValue(0);
  laborOverride_->setValue(-1);
}

int MainWindow::currentWorkOrderRow() const {
  return workOrderTable_->currentRow();
}

void MainWindow::assignWorkOrder() {
  int row = currentWorkOrderRow();
  if (row < 0 || row >= static_cast<int>(store_.workOrders.size())) return;
  auto techId = techSelect_->currentData().toString().toStdString();
  auto techIt = std::find_if(store_.technicians.begin(), store_.technicians.end(), [&](const Technician& t){ return t.id == techId; });
  if (techIt == store_.technicians.end()) return;
  try {
    store_.workOrders[row].assign(*techIt);
  } catch (const std::exception& e) {
    QMessageBox::warning(this, tr("Cannot assign"), e.what());
  }
  populateWorkOrders();
  updateSummary();
}

void MainWindow::startWorkOrder() {
  int row = currentWorkOrderRow();
  if (row < 0 || row >= static_cast<int>(store_.workOrders.size())) return;
  try {
    store_.workOrders[row].start();
  } catch (const std::exception& e) {
    QMessageBox::warning(this, tr("Cannot start"), e.what());
  }
  populateWorkOrders();
  updateSummary();
}

void MainWindow::completeWorkOrder() {
  int row = currentWorkOrderRow();
  if (row < 0 || row >= static_cast<int>(store_.workOrders.size())) return;
  try {
    store_.workOrders[row].complete();
  } catch (const std::exception& e) {
    QMessageBox::warning(this, tr("Cannot complete"), e.what());
  }
  populateWorkOrders();
  updateSummary();
}

void MainWindow::settleWorkOrder() {
  int row = currentWorkOrderRow();
  if (row < 0 || row >= static_cast<int>(store_.workOrders.size())) return;
  try {
    double total = store_.workOrders[row].settle();
    QMessageBox::information(this, tr("Settled"), tr("Total: %1").arg(total));
  } catch (const std::exception& e) {
    QMessageBox::warning(this, tr("Cannot settle"), e.what());
  }
  populateWorkOrders();
  updateSummary();
}

void MainWindow::refreshVehicleList() {
  auto* item = customerList_->currentItem();
  if (!item) return;
  std::string id = item->data(Qt::UserRole).toString().toStdString();
  populateVehiclesForCustomer(id);
}

#include "ui/gui.moc"

