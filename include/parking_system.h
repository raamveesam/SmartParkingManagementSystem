#ifndef PARKING_SYSTEM_H
#define PARKING_SYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define MAX_SLOTS 30
#define SLOTS_PER_FLOOR 10
#define TOTAL_FLOORS 3

#define PLATE_LEN 20
#define NAME_LEN 50
#define PHONE_LEN 20
#define TIME_STR_LEN 30

/* Base Hourly Rates */
#define RATE_TWO_WHEELER   15.00
#define RATE_FOUR_WHEELER  30.00
#define RATE_HEAVY_VEHICLE 60.00
#define MIN_BILLING_HOURS  1.0

/* Vehicle Types */
typedef enum {
    TYPE_TWO_WHEELER = 1,
    TYPE_FOUR_WHEELER = 2,
    TYPE_HEAVY_VEHICLE = 3
} VehicleType;

/* Slot Availability Status */
typedef enum {
    SLOT_VACANT = 0,
    SLOT_OCCUPIED = 1,
    SLOT_MAINTENANCE = 2
} SlotStatus;

/* Vehicle Entity */
typedef struct {
    char licensePlate[PLATE_LEN];
    char ownerName[NAME_LEN];
    char contactNumber[PHONE_LEN];
    VehicleType type;
    time_t registeredAt;
} Vehicle;

/* Linked List Node for Vehicle Registry */
typedef struct VehicleNode {
    Vehicle vehicle;
    struct VehicleNode* next;
} VehicleNode;

/* Parking Slot Entity (Array-backed) */
typedef struct {
    int slotId;             /* 1 to MAX_SLOTS */
    int floor;              /* 1, 2, 3 ... */
    VehicleType supportedType;
    SlotStatus status;
    char occupiedPlate[PLATE_LEN];
    int ticketId;
    time_t entryTime;
} ParkingSlot;

/* Waiting Queue Node (FIFO Queue) */
typedef struct QueueNode {
    Vehicle vehicle;
    time_t requestTime;
    struct QueueNode* next;
} QueueNode;

/* Waiting Queue Structure */
typedef struct {
    QueueNode* front;
    QueueNode* rear;
    int count;
} WaitingQueue;

/* Completed Parking Transaction */
typedef struct {
    int ticketId;
    char licensePlate[PLATE_LEN];
    VehicleType vehicleType;
    int slotId;
    int floor;
    time_t entryTime;
    time_t exitTime;
    double durationHours;
    double baseRate;
    double surgeFactor;
    double totalCharge;
} Transaction;

/* Linked List Node for Historical Transactions */
typedef struct TransactionNode {
    Transaction data;
    struct TransactionNode* next;
} TransactionNode;

/* AI Demand Prediction Model Parameters */
typedef struct {
    double hourlyHistoricalCount[24]; /* Total vehicle arrivals per hour (0-23) */
    double hourlyAverageOccupancy[24];/* Occupancy fraction 0.0 to 1.0 */
    double regressionSlope;            /* OLS slope: demand vs hour */
    double regressionIntercept;        /* OLS intercept */
    bool isTrained;
} AIDemandModel;

/* Overall Parking System Context */
typedef struct {
    ParkingSlot slots[MAX_SLOTS];      /* Array: Fixed parking slot grid */
    int totalSlots;
    VehicleNode* registryHead;         /* Linked List: Registered vehicles */
    WaitingQueue waitingQueue;         /* Queue: Waiting vehicles when full */
    TransactionNode* historyHead;      /* Linked List: Billing transactions */
    int totalTransactions;
    int nextTicketId;
    double totalRevenueCollected;
    AIDemandModel aiModel;             /* AI Demand Predictor */
} ParkingSystem;

/* Helper utility functions */
const char* vehicle_type_to_string(VehicleType type);
const char* slot_status_to_string(SlotStatus status);
void format_timestamp(time_t rawTime, char* buffer, size_t maxLen);
bool plates_match(const char* a, const char* b);

/* System Lifecycle */
void init_parking_system(ParkingSystem* sys);
void free_parking_system(ParkingSystem* sys);

/* Vehicle Registry Module (Linked List) */
bool register_vehicle(ParkingSystem* sys, const char* plate, const char* owner, const char* phone, VehicleType type);
Vehicle* search_registered_vehicle(ParkingSystem* sys, const char* plate);
void display_registered_vehicles(const ParkingSystem* sys);
void free_vehicle_registry(ParkingSystem* sys);

/* Parking Allocation Module (Array) */
void init_parking_slots(ParkingSystem* sys);
int allocate_slot_for_vehicle(ParkingSystem* sys, VehicleType type, const char* plate, int ticketId, time_t entryTime);
bool free_slot(ParkingSystem* sys, int slotId);
int find_slot_by_plate(const ParkingSystem* sys, const char* plate);
int count_available_slots_by_type(const ParkingSystem* sys, VehicleType type);
void display_slots_table(const ParkingSystem* sys);
void display_parking_grid_ascii(const ParkingSystem* sys);

/* Waiting Queue Module (FIFO Queue) */
void init_waiting_queue(WaitingQueue* q);
bool enqueue_waiting_vehicle(WaitingQueue* q, Vehicle vehicle);
bool dequeue_waiting_vehicle(WaitingQueue* q, Vehicle* outVehicle);
bool peek_waiting_vehicle(const WaitingQueue* q, Vehicle* outVehicle);
void display_waiting_queue(const WaitingQueue* q);
void free_waiting_queue(WaitingQueue* q);

/* Entry & Exit Tracking */
bool process_vehicle_entry(ParkingSystem* sys, const char* plate, const char* owner, const char* phone, VehicleType type, time_t customEntry);
bool process_vehicle_exit(ParkingSystem* sys, const char* plate, time_t customExit);

/* Billing Module */
double get_hourly_rate(VehicleType type);
double calculate_parking_charges(VehicleType type, double durationHours, double surgeFactor);
void add_transaction_to_history(ParkingSystem* sys, Transaction tx);
void display_transaction_history(const ParkingSystem* sys);
void print_bill_receipt(const Transaction* tx);
void free_transaction_history(ParkingSystem* sys);

/* Searching and Sorting Module (DSA Extension) */
int linear_search_occupied_slot(const ParkingSystem* sys, const char* plate);
Transaction** get_transactions_array(const ParkingSystem* sys, int* outCount);
void quick_sort_transactions_by_amount(Transaction** arr, int low, int high, bool ascending);
void merge_sort_transactions_by_time(Transaction** arr, int left, int right);
int binary_search_transaction_by_ticket(Transaction** sortedArr, int count, int ticketId);

/* AI Demand Prediction Module */
void train_ai_demand_model(ParkingSystem* sys);
void predict_demand_for_hour(const ParkingSystem* sys, int hour, double* outDemandPct, double* outSurgeMultiplier);
void display_ai_demand_dashboard(const ParkingSystem* sys);

/* Parking Statistics */
void display_parking_statistics(const ParkingSystem* sys);

/* File Persistence */
bool save_system_state(const ParkingSystem* sys, const char* filename);
bool load_system_state(ParkingSystem* sys, const char* filename);

/* Automated Demo Mode */
void run_automated_demo(ParkingSystem* sys);

#endif /* PARKING_SYSTEM_H */
