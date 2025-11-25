# Auto Repair Plant Management System

A auto-repair management demo that now includes an automatic **diagnostic/detection** step for every work order and a capacity-aware **storehouse** that tracks and alerts on part consumption.

## UML diagram
The diagram is organized by [Graphviz](https://graphviz.org/) using **.dot** language. 
Here is a [online editor](https://dreampuf.github.io/GraphvizOnline/?engine=dot#digraph%20G%20%7B%0A%0A%20%20subgraph%20cluster_0%20%7B%0A%20%20%20%20style%3Dfilled%3B%0A%20%20%20%20color%3Dlightgrey%3B%0A%20%20%20%20node%20%5Bstyle%3Dfilled%2Ccolor%3Dwhite%5D%3B%0A%20%20%20%20a0%20-%3E%20a1%20-%3E%20a2%20-%3E%20a3%3B%0A%20%20%20%20label%20%3D%20%22process%20%231%22%3B%0A%20%20%7D%0A%0A%20%20subgraph%20cluster_1%20%7B%0A%20%20%20%20node%20%5Bstyle%3Dfilled%5D%3B%0A%20%20%20%20b0%20-%3E%20b1%20-%3E%20b2%20-%3E%20b3%3B%0A%20%20%20%20label%20%3D%20%22process%20%232%22%3B%0A%20%20%20%20color%3Dblue%0A%20%20%7D%0A%20%20start%20-%3E%20a0%3B%0A%20%20start%20-%3E%20b0%3B%0A%20%20a1%20-%3E%20b3%3B%0A%20%20b2%20-%3E%20a3%3B%0A%20%20a3%20-%3E%20a0%3B%0A%20%20a3%20-%3E%20end%3B%0A%20%20b3%20-%3E%20end%3B%0A%0A%20%20start%20%5Bshape%3DMdiamond%5D%3B%0A%20%20end%20%5Bshape%3DMsquare%5D%3B%0A%7D).

## Quick Start

### Requirements
- CMake 3.12+
- A C++17 compiler
- Qt 6 (Widgets module)

### Build & Run
```bash
cd auto-repair
cmake -S . -B build
cmake --build build -j
./build/auto_repair
```
The first launch seeds `data/data_store.json` and `data/parts.csv` with sample customers, vehicles, and a stocked inventory.

## Data & Persistence
- **Storehouse inventory** is loaded from/saved to `data/data_store.json` (runtime state) and `data/parts.csv` (seed data).
- Each part now carries `capacity`; alerts fire when stock drops to **≤ 10% of capacity** (or below its reorder point if capacity is 0).
- Work orders persist detected service notes, assigned parts, and pricing strategy choices.

## Work Order Detection & Inventory Flow
1. **Detection step**: when creating a work order, the system inspects the vehicle (year/brand) and proposes service items plus the required parts (oil/filter, air filter, brake pads, or a fallback inspection). A dialog lists what was detected; you can still add a manual service line.
2. **Assignment & progress**: Draft → Assigned → In Progress → Completed → Paid.
3. **Settlement & stock**: Settling consumes the required parts from the storehouse. If quantities are insufficient, settlement warns you. When stock drops under 10% of capacity, alerts accompany the settlement dialog. Inventory snapshots show updated stock and capacity.

## GUI User Manual
Launch `./build/auto_repair` to open the Qt GUI. Use the tabs at the top to navigate:

### Customers
- Left pane: pick an existing customer.
- Right pane: manage the customer’s vehicles (VIN, plate, brand, model, year).
- Use **Add Customer** / **Add Vehicle** to expand the roster.

### Work Orders
- Choose a vehicle and mechanic.
- Press **Create Work Order**. The detector proposes service/parts automatically and shows them in a popup. Optionally fill in the manual service fields (ID, name, labor hours, base price, labor override) to append a custom item.
- Use **Assign**, **Start**, **Complete**, and **Settle** to move through the lifecycle.
- Settlement consumes parts, persists the updated storehouse, and surfaces low-stock alerts (capacity-based).
- The table lists each order’s ID, vehicle plate, customer, mechanic, status, total, and the detection note.

### Mechanics
- Add technicians with ID, name, and hourly rate.
- View assigned work orders per technician.

### Summary
- Shows counts and IDs per status (Draft/Paid/etc.).
- Displays the **inventory snapshot** with current stock vs. capacity so you can plan replenishment.

## CLI Demo (optional)
`src/ui/cli.cpp` still contains a console demo that seeds inventory, runs detection, and prints alerts; the GUI is the primary interface.
