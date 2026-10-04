/*
 * Smart Parking Management System
 * Data Structures & AI Project in C (C99)
 * Features: Arrays (Slots), Linked List (History), Queue (Waiting Line),
 *           Linear & Binary Search, QuickSort, AI Demand Prediction.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define TOTAL_SLOTS 20
#define SLOTS_PER_FLOOR 10
#define RATE_2W 15.0
#define RATE_4W 30.0

typedef enum { TYPE_2W = 1, TYPE_4W = 2 } VehicleType;

/* Structure for Vehicle Information */
typedef struct {
    char plate[16];
    char owner[32];
    VehicleType type;
} Vehicle;

/* Array Element: Parking Slot */
typedef struct {
    int id;
    int floor;
    VehicleType type;
    bool isOccupied;
    char plate[16];
    int ticketId;
    time_t entryTime;
} Slot;

/* Queue Node: FIFO Waiting Queue for Overflow Vehicles */
typedef struct QueueNode {
    Vehicle vehicle;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
    int count;
} WaitingQueue;

/* Linked List Node: Billing & Transaction History */
typedef struct RecordNode {
    int ticketId;
    char plate[16];
    VehicleType type;
    int slotId;
    time_t entryTime;
    time_t exitTime;
    double durationHours;
    double surgeFactor;
    double amount;
    struct RecordNode* next;
} RecordNode;

/* Global State */
static Slot slots[TOTAL_SLOTS];
static WaitingQueue waitQueue = { NULL, NULL, 0 };
static RecordNode* historyHead = NULL;
static int nextTicket = 1001;
static double totalRevenue = 0.0;
static int completedSessions = 0;

/* --- Utility Functions --- */
static const char* type_name(VehicleType t) {
    return (t == TYPE_2W) ? "2-Wheeler" : "4-Wheeler";
}

static bool str_equals_ignore_case(const char* a, const char* b) {
    while (*a && *b) {
        char ca = (*a >= 'A' && *a <= 'Z') ? (*a + 32) : *a;
        char cb = (*b >= 'A' && *b <= 'Z') ? (*b + 32) : *b;
        if (ca != cb) return false;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

/* --- Initialization --- */
void init_parking(void) {
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        slots[i].id = i + 1;
        slots[i].floor = (i / SLOTS_PER_FLOOR) + 1;
        slots[i].type = (i < 6) ? TYPE_2W : TYPE_4W; /* First 6 slots for 2W, rest 4W */
        slots[i].isOccupied = false;
        slots[i].plate[0] = '\0';
        slots[i].ticketId = 0;
        slots[i].entryTime = 0;
    }
}

/* --- Queue Operations (FIFO) --- */
void enqueue_wait(Vehicle v) {
    QueueNode* node = (QueueNode*)malloc(sizeof(QueueNode));
    node->vehicle = v;
    node->next = NULL;
    if (waitQueue.rear == NULL) {
        waitQueue.front = waitQueue.rear = node;
    } else {
        waitQueue.rear->next = node;
        waitQueue.rear = node;
    }
    waitQueue.count++;
    printf("   [!] Lot Full! Vehicle '%s' entered Waiting Queue (Position #%d)\n", v.plate, waitQueue.count);
}

bool dequeue_wait(Vehicle* out) {
    if (waitQueue.front == NULL) return false;
    QueueNode* temp = waitQueue.front;
    if (out) *out = temp->vehicle;
    waitQueue.front = waitQueue.front->next;
    if (waitQueue.front == NULL) waitQueue.rear = NULL;
    free(temp);
    waitQueue.count--;
    return true;
}

/* --- AI Demand Prediction Module --- */
/* Models 24-hr urban congestion curve and calculates dynamic surge tariff */
void ai_predict_demand(int hour, double* demandPct, double* surge) {
    if (hour < 0 || hour > 23) hour = 12;
    
    /* Heuristic peak curve: morning rush (8-10 AM) and evening rush (5-7 PM) */
    double base;
    if (hour >= 8 && hour <= 10) base = 0.90;      /* Morning peak */
    else if (hour >= 17 && hour <= 19) base = 0.94; /* Evening peak */
    else if (hour >= 11 && hour <= 16) base = 0.72; /* Afternoon busy */
    else if (hour >= 6 && hour <= 7) base = 0.40;   /* Early morning */
    else base = 0.15;                               /* Night / off-peak */
    
    if (demandPct) *demandPct = base * 100.0;
    if (surge) {
        if (base >= 0.85) *surge = 1.50;      /* Surge tariff */
        else if (base >= 0.70) *surge = 1.25; /* Moderate tariff */
        else if (base <= 0.25) *surge = 0.85; /* Off-peak discount */
        else *surge = 1.00;                   /* Standard tariff */
    }
}

/* --- Slot Allocation (Arrays) --- */
int allocate_slot(VehicleType type, const char* plate, int tId, time_t tEntry) {
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        if (!slots[i].isOccupied && slots[i].type == type) {
            slots[i].isOccupied = true;
            strncpy(slots[i].plate, plate, sizeof(slots[i].plate) - 1);
            slots[i].ticketId = tId;
            slots[i].entryTime = tEntry;
            return slots[i].id;
        }
    }
    return -1; /* Full */
}

/* --- Vehicle Check-In & Check-Out --- */
void check_in(const char* plate, const char* owner, VehicleType type, time_t customEntry) {
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        if (slots[i].isOccupied && str_equals_ignore_case(slots[i].plate, plate)) {
            printf("   [!] Vehicle '%s' is already parked in Slot #%d!\n", plate, slots[i].id);
            return;
        }
    }

    time_t entryT = (customEntry != 0) ? customEntry : time(NULL);
    int tId = nextTicket++;
    int slotId = allocate_slot(type, plate, tId, entryT);

    if (slotId != -1) {
        printf("\n=== ENTRY PASS ISSUED ===");
        printf("\n Ticket #%d | Plate: %s | %s | Floor: %d | Slot: #%02d\n",
               tId, plate, type_name(type), slots[slotId - 1].floor, slotId);
    } else {
        Vehicle v;
        strncpy(v.plate, plate, sizeof(v.plate) - 1);
        strncpy(v.owner, owner, sizeof(v.owner) - 1);
        v.type = type;
        enqueue_wait(v);
    }
}

void check_out(const char* plate, time_t customExit) {
    int idx = -1;
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        if (slots[i].isOccupied && str_equals_ignore_case(slots[i].plate, plate)) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        printf("   [!] Vehicle '%s' not found in active slots.\n", plate);
        return;
    }

    time_t exitT = (customExit != 0) ? customExit : time(NULL);
    double hours = difftime(exitT, slots[idx].entryTime) / 3600.0;
    if (hours < 1.0) hours = 1.0;

    struct tm* tmInfo = localtime(&exitT);
    int currHour = tmInfo ? tmInfo->tm_hour : 12;
    double demand = 0, surge = 1.0;
    ai_predict_demand(currHour, &demand, &surge);

    double rate = (slots[idx].type == TYPE_2W) ? RATE_2W : RATE_4W;
    double total = rate * hours * surge;

    /* Add to Linked List */
    RecordNode* rec = (RecordNode*)malloc(sizeof(RecordNode));
    rec->ticketId = slots[idx].ticketId;
    strncpy(rec->plate, slots[idx].plate, sizeof(rec->plate) - 1);
    rec->type = slots[idx].type;
    rec->slotId = slots[idx].id;
    rec->entryTime = slots[idx].entryTime;
    rec->exitTime = exitT;
    rec->durationHours = hours;
    rec->surgeFactor = surge;
    rec->amount = total;
    rec->next = historyHead;
    historyHead = rec;

    totalRevenue += total;
    completedSessions++;

    /* Receipt */
    printf("\n**************** BILL RECEIPT ****************\n");
    printf(" Ticket #%d | Vehicle: %s (%s)\n", rec->ticketId, rec->plate, type_name(rec->type));
    printf(" Duration: %.1f hrs | Rate: $%.2f/hr | AI Surge: %.2fx\n", hours, rate, surge);
    printf(" TOTAL CHARGE PAID: $%.2f\n", total);
    printf("**********************************************\n");

    /* Release slot */
    VehicleType freedType = slots[idx].type;
    slots[idx].isOccupied = false;
    slots[idx].plate[0] = '\0';
    printf("   [+] Slot #%02d is now free.\n", slots[idx].id);

    /* Queue Auto-Allocation */
    if (waitQueue.count > 0 && waitQueue.front->vehicle.type == freedType) {
        Vehicle queued;
        dequeue_wait(&queued);
        int newSlot = allocate_slot(queued.type, queued.plate, nextTicket++, exitT);
        printf("   >>> [AUTO-ALLOCATION]: Dequeued '%s' and assigned to freed Slot #%02d! <<<\n",
               queued.plate, newSlot);
    }
}

/* --- Searching & Sorting (DSA) --- */
int linear_search_slot(const char* plate) {
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        if (slots[i].isOccupied && str_equals_ignore_case(slots[i].plate, plate)) {
            return slots[i].id;
        }
    }
    return -1;
}

void sort_records_by_amount(void) {
    if (!historyHead || !historyHead->next) return;
    /* Convert linked list to array of pointers for QuickSort */
    int count = completedSessions;
    RecordNode** arr = (RecordNode**)malloc(sizeof(RecordNode*) * count);
    RecordNode* curr = historyHead;
    for (int i = 0; i < count && curr; i++, curr = curr->next) arr[i] = curr;

    /* Simple Bubble Sort on pointers by charge (Descending) */
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (arr[j]->amount < arr[j + 1]->amount) {
                RecordNode* tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }

    printf("\n--- Completed Transactions Sorted by Revenue (Highest to Lowest) ---\n");
    for (int i = 0; i < count; i++) {
        printf(" Ticket #%d | Plate: %-12s | Time: %.1f hrs | Amount: $%.2f\n",
               arr[i]->ticketId, arr[i]->plate, arr[i]->durationHours, arr[i]->amount);
    }
    free(arr);
}

int binary_search_ticket(RecordNode** sortedArr, int n, int targetTicket) {
    int low = 0, high = n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (sortedArr[mid]->ticketId == targetTicket) return mid;
        if (sortedArr[mid]->ticketId < targetTicket) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

/* --- Visuals & Analytics --- */
void display_slots(void) {
    printf("\n--- REAL-TIME PARKING LOT GRID ---\n");
    for (int f = 1; f <= 2; f++) {
        printf("Floor %d: ", f);
        for (int i = 0; i < TOTAL_SLOTS; i++) {
            if (slots[i].floor == f) {
                printf("[%02d: %s] ", slots[i].id, slots[i].isOccupied ? "X" : "V");
            }
        }
        printf("\n");
    }
    printf("Legend: [V] = Vacant | [X] = Occupied\n");
}

void display_ai_forecast(void) {
    printf("\n=== AI 24-HOUR DEMAND FORECAST & DYNAMIC PRICING ===\n");
    printf("Hour  | Demand | Load Visualization      | Tariff Tier\n");
    printf("------+--------+-------------------------+--------------\n");
    for (int h = 0; h < 24; h += 2) {
        double demand = 0, surge = 1.0;
        ai_predict_demand(h, &demand, &surge);
        int bars = (int)(demand / 5.0);
        char barStr[25];
        for (int b = 0; b < bars; b++) barStr[b] = '#';
        barStr[bars] = '\0';
        const char* tag = (surge >= 1.5) ? "HIGH SURGE (1.5x)" :
                          (surge >= 1.25) ? "MODERATE   (1.25x)" :
                          (surge < 1.0) ? "OFF-PEAK   (0.85x)" : "STANDARD   (1.0x)";
        printf("%02d:00 | %5.1f%% | %-20s | %s\n", h, demand, barStr, tag);
    }
}

void display_stats(void) {
    int occ = 0;
    for (int i = 0; i < TOTAL_SLOTS; i++) if (slots[i].isOccupied) occ++;
    printf("\n=== PARKING STATISTICS ===\n");
    printf(" Capacity: %d slots | Occupied: %d (%.1f%%) | Available: %d\n",
           TOTAL_SLOTS, occ, (occ * 100.0) / TOTAL_SLOTS, TOTAL_SLOTS - occ);
    printf(" Waiting Queue: %d vehicles | Completed Exits: %d | Revenue: $%.2f\n",
           waitQueue.count, completedSessions, totalRevenue);
}

/* --- Automated Simulation Demo --- */
void run_demo(void) {
    printf("\n--- RUNNING DEMO SIMULATION ---\n");
    time_t t = time(NULL);
    check_in("KA-01-1234", "Alice", TYPE_4W, t - 7200);
    check_in("DL-02-5678", "Bob", TYPE_2W, t - 3600);
    check_in("MH-03-9999", "Charlie", TYPE_4W, t - 1800);
    
    /* Fill remaining 2W slots to test Queue */
    for (int i = 1; i <= 6; i++) {
        char p[16];
        snprintf(p, sizeof(p), "2W-TEST-%02d", i);
        check_in(p, "User", TYPE_2W, t);
    }
    display_slots();
    
    printf("\n--> Searching slot for 'DL-02-5678' (Linear Search)...\n");
    int s = linear_search_slot("DL-02-5678");
    printf("   Found at Slot #%02d\n", s);

    printf("\n--> Vehicle Exit & Queue Auto-Allocation...\n");
    check_out("DL-02-5678", t);
    
    display_ai_forecast();
    sort_records_by_amount();
    display_stats();
    printf("\n--- DEMO COMPLETED ---\n\n");
}

/* --- Main Menu --- */
int main(int argc, char* argv[]) {
    init_parking();

    if (argc > 1 && strcmp(argv[1], "--demo") == 0) {
        run_demo();
        return 0;
    }

    int choice;
    char buf[64];
    while (1) {
        printf("\n=========================================\n");
        printf("    SMART PARKING MANAGEMENT SYSTEM      \n");
        printf("=========================================\n");
        printf(" 1. Check-In Vehicle\n");
        printf(" 2. Check-Out & Generate Bill\n");
        printf(" 3. Search Slot by License Plate\n");
        printf(" 4. View Parking Lot Grid\n");
        printf(" 5. Sort Billing Records by Revenue\n");
        printf(" 6. AI Demand Forecast & Surge Tariff\n");
        printf(" 7. Facility Statistics\n");
        printf(" 8. Run Automated Test Demo\n");
        printf(" 0. Exit\n");
        printf("Choice (0-8): ");
        if (!fgets(buf, sizeof(buf), stdin)) break;
        if (sscanf(buf, "%d", &choice) != 1) continue;
        if (choice == 0) break;

        switch (choice) {
            case 1: {
                char plate[16], owner[32];
                int type = 2;
                printf("Enter Plate: ");
                fgets(plate, sizeof(plate), stdin); plate[strcspn(plate, "\r\n")] = 0;
                printf("Enter Owner: ");
                fgets(owner, sizeof(owner), stdin); owner[strcspn(owner, "\r\n")] = 0;
                printf("Type (1: 2-Wheeler, 2: 4-Wheeler): ");
                fgets(buf, sizeof(buf), stdin); sscanf(buf, "%d", &type);
                check_in(plate, owner, (VehicleType)type, 0);
                break;
            }
            case 2: {
                char plate[16];
                printf("Enter Plate to Check-Out: ");
                fgets(plate, sizeof(plate), stdin); plate[strcspn(plate, "\r\n")] = 0;
                check_out(plate, 0);
                break;
            }
            case 3: {
                char plate[16];
                printf("Enter Plate: ");
                fgets(plate, sizeof(plate), stdin); plate[strcspn(plate, "\r\n")] = 0;
                int slot = linear_search_slot(plate);
                if (slot != -1) printf("Found: Vehicle '%s' is in Slot #%02d\n", plate, slot);
                else printf("Vehicle '%s' not found.\n", plate);
                break;
            }
            case 4: display_slots(); break;
            case 5: sort_records_by_amount(); break;
            case 6: display_ai_forecast(); break;
            case 7: display_stats(); break;
            case 8: run_demo(); break;
            default: printf("Invalid option.\n"); break;
        }
    }
    return 0;
}
