
# Auto Repair Plant Management System (C++17 / CMake)

A small, object‑oriented **Auto Repair Plant Management System** for teaching purposes (fits the “面向对象程序设计开发与实现” experiment).  
It demonstrates **encapsulation, inheritance, polymorphism, composition, strategy, observer, state machine**, and simple **CSV persistence**.

## Features

- **Customer & Vehicle** management (simplified in memory)
- **Work Order lifecycle**: Draft → Assigned → InProgress → Completed → Paid
- **Pricing strategies (Strategy Pattern)**: normal / member / campaign
- **Employees (Inheritance + Polymorphism)**: Technician, ServiceAdvisor, Manager
- **Inventory (Observer Pattern)**: low‑stock notification
- **Simple persistence**: CSV repository for Parts
- **Reports**: turnover of paid orders (toy demo)

## Project Layout

```
auto_repair/
├─ CMakeLists.txt
├─ data/
│  └─ parts.csv              # sample data (auto-created/updated)
└─ src/
   ├─ main.cpp
   ├─ domain/
   │  ├─ person.hpp
   │  ├─ employee.hpp
   │  ├─ customer.hpp
   │  ├─ vehicle.hpp
   │  ├─ part.hpp
   │  ├─ service_item.hpp
   │  ├─ work_order.hpp
   │  └─ work_order.cpp
   ├─ inventory/
   │  ├─ observer.hpp
   │  └─ inventory.hpp
   ├─ persistence/
   │  ├─ repository.hpp
   │  └─ file_repository.hpp
   ├─ report/
   │  └─ report_service.hpp
   └─ ui/
      ├─ cli.hpp
      └─ cli.cpp
```

## Build & Run

```bash
# 1) Configure and build (out-of-source)
cd auto_repair
cmake -S . -B build
cmake --build build -j

# 2) Run
./build/auto_repair
```

Expected output (similar to):

```
WorkOrder WO0001 settled. Total: 189
[ALERT] Part P002 low stock: 1
Paid Orders: 1, Turnover: 189
```

> The **[ALERT]** line appears when a part stock is at/under its reorder threshold.

## How It Works (Flow)

1. **Seed Data**: Parts are read/written via a tiny CSV repository (`PartCsvRepository`).  
2. **Create Actors**: Customer, Vehicle, Technician (hourly), ServiceAdvisor (salary+commission).  
3. **Create Work Order** (`WO0001`), add one service item (“更换机油”) consuming two parts.  
4. **Pricing Strategy**: if customer level is VIP, we switch to `MemberDiscountPricing(0.9)`.  
5. **Lifecycle**: `assign()` → `start()` → `complete()` → `settle()` → status becomes `Paid`.  
6. **Inventory**: parts are consumed and the observer prints a low‑stock **ALERT** when needed.  
7. **Report**: Turnover sums totals of paid orders.

## OOP Showcases

- **Inheritance**: `Employee -> Technician / ServiceAdvisor / Manager`
- **Polymorphism**: `Employee*` calling `calculatePay()` uses the concrete subclass at runtime;  
  `PricingStrategy` pointers bind to `NormalPricing`/`MemberDiscountPricing`/`CampaignPricing`.
- **Strategy Pattern**: pricing is pluggable via `PricingStrategy`.
- **Observer Pattern**: `InventoryObserver` → `ReorderNotifier` for low stock.
- **State Machine**: guarded transitions in `WorkOrder` (throws on invalid order).

## Extend Ideas

- Add CRUD menus and repositories for customers/vehicles/work orders.
- Add `CampaignPricing` time‑window logic.
- Replace CSV with SQLite/JSON/YAML as you like.
- Add more reports (top parts, technician utilization, etc.).

## License

Educational use; you can adapt freely in coursework.
