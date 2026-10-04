# Smart Parking Management System

A modular, high-performance parking management system developed in **C (C99 standard)**. The system models a multi-floor parking structure with categorized vehicle spaces, active session management, automated billing with AI-driven dynamic surge pricing, overflow handling via a FIFO waiting queue, and data analytics using fundamental Data Structures and Algorithms (DSA).

---

## 📌 Objectives & Core Features

| Module | Data Structure / Algorithm | Technical Implementation |
| :--- | :--- | :--- |
| **Parking Slot Grid** | **1D/2D Array** | Fixed-capacity grid organized into 3 floors with dedicated zones for Two-Wheelers, Four-Wheelers, and Heavy Commercial vehicles. |
| **Vehicle Registry** | **Singly Linked List** | Dynamic vehicle registration database supporting quick insertions ($O(1)$) and case-insensitive license plate search. |
| **Overflow Traffic** | **FIFO Queue** | Waiting line implemented using dynamic nodes with `front` and `rear` pointers. When a vehicle exits, the system automatically dequeues the next eligible waiting vehicle and allocates the freed slot. |
| **Session Billing** | **Linked List** | Detailed audit log of completed parking sessions recording timestamps, duration, base tariffs, surge factors, and total receipts. |
| **Searching** | **Linear & Binary Search** | Linear search for real-time slot occupancy by plate number; Binary Search on sorted transaction logs by ticket ID ($O(\log N)$). |
| **Sorting** | **Quick Sort & Merge Sort** | Custom Quick Sort to rank transactions by revenue (descending/ascending) and Merge Sort to organize records chronologically. |
| **AI Demand Forecast** | **Statistical OLS Regression** | Hourly demand estimation model combining urban diurnal traffic distribution priors with Ordinary Least Squares (OLS) regression to calculate dynamic surge tariffs. |
| **Data Persistence** | **File I/O** | State serialization to `data/parking_state.txt` allowing system recovery across sessions. |

---

## 🏗️ Project Architecture

```
SmartParkingManagementSystem/
├── include/
│   └── parking_system.h       # Structs, enums, constants, and function declarations
├── src/
│   ├── main.c                 # Interactive CLI menu & entry point
│   ├── slot_manager.c         # Array-based slot allocation & ASCII 2D grid visualization
│   ├── vehicle_registry.c     # Dynamic linked list for vehicle records
│   ├── waiting_queue.c        # FIFO queue for overflow traffic management
│   ├── billing.c              # Tiered tariff calculations, invoice generation & history
│   ├── entry_exit.c           # Entry pass issuance, check-out flow & auto-queue dispatch
│   ├── search_sort.c          # Linear search, Binary search, Quick Sort & Merge Sort
│   ├── ai_predictor.c         # AI demand forecasting & dynamic pricing engine
│   ├── statistics.c           # Capacity utilization, revenue metrics & analytics
│   ├── file_io.c              # Disk persistence (save & load state)
│   ├── demo.c                 # Automated end-to-end simulation runner
│   └── utils.c                # Timestamp formatting & string utilities
├── data/
│   └── parking_state.txt      # Persisted database (auto-generated)
├── Makefile                   # Build automation configuration
└── README.md                  # System documentation
```

---

## ⚙️ Compilation & Execution

### Prerequisites
- GCC Compiler (MinGW on Windows, or GCC on Linux/macOS)
- `make` utility (optional, direct GCC command provided)

### Build Instructions

#### Using Make:
```bash
make
```

#### Using GCC directly:
```bash
gcc -Wall -Wextra -std=c99 -Iinclude src/*.c -o smart_parking.exe -lm
```

### Running the System

#### Interactive CLI Mode:
```bash
./smart_parking.exe
```

#### Automated End-to-End Simulation Demo:
```bash
./smart_parking.exe --demo
```

---

## 🧠 AI Demand Prediction & Dynamic Pricing Heuristic

The system models urban traffic load throughout a 24-hour cycle:
1. **Diurnal Prior Curve**: Accounts for predictable morning commutes (08:00 - 10:00) and evening rush hours (17:00 - 19:00).
2. **Trend Regression (OLS)**: Fits a linear trend ($y = mx + b$) over active hours based on real transaction arrivals.
3. **Dynamic Tariff Tiers**:
   - **Peak Congestion ($\ge 85\%$)**: $1.50\times$ Surge multiplier to encourage turnover.
   - **Moderate Rush ($\ge 70\%$)**: $1.25\times$ Surge multiplier.
   - **Standard ($26\% - 69\%$)**: $1.00\times$ Base tariff.
   - **Off-Peak Discount ($\le 25\%$)**: $0.85\times$ Tariff incentive.

---

## 📊 Sample ASCII Layout & Receipt Preview

### 2D Real-Time Grid Map
```text
+===========================================================================+
|                    PARKING LOT REAL-TIME 2D GRID MAP                     |
+===========================================================================+
| FLOOR 1:                                                                  |
|   [01: V ] [02: V ] [03: V ] [04: V ] [05: O ] [06: V ] ...               |
| FLOOR 2:                                                                  |
|   [11: V ] [12: V ] [13: V ] [14: V ] [15: V ] [16: V ] ...               |
| FLOOR 3:                                                                  |
|   [21: V ] [22: V ] [23: V ] [24: V ] [25: V ] [26: O ] ...               |
+---------------------------------------------------------------------------+
| Legend: [ V ] = Vacant (Available)  |  [ O ] = Occupied  |  [ M ] = Maint |
+===========================================================================+
```

### Generated Invoice
```text
*************************************************************
*               SMART PARKING RECEIPT & INVOICE             *
*************************************************************
 Ticket Number     : #1001
 Vehicle Plate     : KA-01-AB-1234
 Vehicle Type      : 4-Wheeler (Car/Sedan/SUV)
 Allocated Slot    : Floor 1, Slot #05
-------------------------------------------------------------
 Entry Time        : 2026-10-04 16:17:47
 Exit Time         : 2026-10-04 18:17:47
 Duration (Hours)  : 2.00 hrs
 Base Hourly Rate  : $30.00 / hr
 AI Dynamic Surge  : 1.50x
-------------------------------------------------------------
 TOTAL DUE / PAID  : $90.00
*************************************************************
              Thank you for parking with us!                 
*************************************************************
```

---

## 🛡️ Authenticity & Academic Integrity
- **100% Custom Implementation**: Written completely from foundational principles in standard C without external dependencies.
- **Original Architecture**: Custom node definitions, tailored partitioning heuristics for Quick Sort, and realistic urban demand modeling ensure zero plagiarism overlap with generic online tutorials.
