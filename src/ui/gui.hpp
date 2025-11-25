#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <vector>
#include "domain/work_order.hpp"

struct DataStore {
  std::vector<Customer> customers;
  std::vector<Vehicle> vehicles;
  std::vector<Technician> technicians;
  std::vector<WorkOrder> workOrders;
};

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(DataStore store, QWidget* parent = nullptr);

private slots:
  void addCustomer();
  void addVehicle();
  void addTechnician();
  void createWorkOrder();
  void assignWorkOrder();
  void startWorkOrder();
  void completeWorkOrder();
  void settleWorkOrder();
  void refreshVehicleList();

private:
  DataStore store_;
  QTabWidget* tabs_{};

  // Customer page widgets
  QListWidget* customerList_{};
  QTableWidget* vehicleTable_{};
  QLineEdit *customerName_{}, *customerPhone_{};
  QComboBox* customerLevel_{};
  QLineEdit *vin_{}, *plate_{}, *brand_{}, *model_{};
  QSpinBox* year_{};

  // Mechanic page widgets
  QTableWidget* technicianTable_{};
  QLineEdit *techId_{}, *techName_{};
  QSpinBox* hourlyRate_{};

  // Work order page widgets
  QTableWidget* workOrderTable_{};
  QComboBox *vehicleSelect_{}, *techSelect_{};
  QLineEdit *serviceId_{}, *serviceName_{};
  QDoubleSpinBox *laborHours_{}, *basePrice_{}, *laborOverride_{};

  // Summary page widgets
  QLabel *summaryLabel_{};
  QTableWidget* statusTable_{};

  void setupUI();
  QWidget* buildCustomerPage();
  QWidget* buildWorkOrderPage();
  QWidget* buildTechnicianPage();
  QWidget* buildSummaryPage();

  void populateCustomers();
  void populateVehiclesForCustomer(const std::string& customerId);
  void populateTechnicians();
  void populateWorkOrders();
  void populateSelectors();
  void updateSummary();
  int currentWorkOrderRow() const;
};

