# Smart Parking Management System

A concise, clean, and complete parking management system written in **C (C99 standard)** (~310 lines of code). The project demonstrates fundamental Data Structures & Algorithms (DSA) combined with an Artificial Intelligence (AI) demand prediction heuristic.

---

## 📌 Features & Data Structures (DSA)

| Feature | Data Structure / Algorithm | Description |
| :--- | :--- | :--- |
| **Parking Grid** | **Array** (`Slot slots[20]`) | 2-floor parking lot layout with dedicated zones for 2-Wheelers and 4-Wheelers. |
| **Overflow Line** | **FIFO Queue** (`WaitingQueue`) | Linked-node queue with `front` and `rear` pointers for waiting vehicles when the lot is full. |
| **Billing History** | **Linked List** (`RecordNode`) | Dynamic transaction log tracking tickets, durations, tariffs, and receipts. |
| **Slot Allocation** | **Array Search & Dispatch** | Assigns nearest available slot; auto-dequeues and assigns waiting vehicles when a slot is freed. |
| **Searching** | **Linear & Binary Search** | Linear search by license plate; Binary search on sorted ticket IDs. |
| **Sorting** | **Sorting Algorithm** | Ranks completed parking sessions by total revenue paid. |
| **AI Extension** | **Demand Curve & Surge Pricing** | Models 24-hour urban traffic congestion and adjusts tariffs dynamically (0.85x to 1.50x). |

---

## 🚀 How to Build and Run

### 1. Compile with GCC
```bash
gcc -Wall -Wextra -std=c99 main.c -o smart_parking.exe -lm
```
*(or run `make`)*

### 2. Run Automated Demo Simulation
```bash
./smart_parking.exe --demo
```

### 3. Run Interactive Menu
```bash
./smart_parking.exe
```

---

## 📂 Project Structure

```
SmartParkingManagementSystem/
├── main.c                                  # Complete, self-contained C implementation (~310 lines)
├── Makefile                                # Build automation script
├── README.md                               # Project documentation
├── Smart_Parking_Management_System_Code.docx# Formatted Word document for submission
└── generate_docx.py                        # Script to generate Word doc
```
