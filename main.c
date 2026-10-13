/* Smart Parking Management System


   Uses: array (slots, history), linked list (vehicle records), queue (waiting line),
   searching (plate lookup, nearest slot), sorting (revenue report), demand prediction. */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SLOTS 8
#define QSIZE 4
#define DAYS  3

typedef struct Record {
    char plate[16];
    int type;                 /* 0 = bike, 1 = car, 2 = truck */
    int slot;                 /* -1 while waiting in queue    */
    int inHour, outHour;
    int fee;
    int active;               /* 1 = parked or waiting, 0 = left */
    struct Record *next;
} Record;

static const char *typeName[] = {"Bike", "Car", "Truck"};
static const int   rate[]     = {10, 20, 40};            /* Rs per hour */
static const int   pattern[24] = {0,0,0,0,0,1,1,2,4,6,7,5,4,4,3,3,4,6,7,6,4,2,1,0};

Record *head = NULL;              /* linked list of all vehicle records */
Record *slots[SLOTS];             /* array: slot -> vehicle (NULL = free) */
Record *waitQ[QSIZE];             /* circular queue of waiting vehicles  */
int qFront = 0, qCount = 0;
int hist[DAYS][24], today[24];    /* entries per hour: past days, today  */

/* ---------- helpers ---------- */
int readInt(const char *msg, int lo, int hi) {
    int v;
    for (;;) {
        printf("%s", msg);
        if (scanf("%d", &v) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) ;
            if (c == EOF) exit(0);
        } else if (v >= lo && v <= hi) return v;
        printf("  Enter a value between %d and %d.\n", lo, hi);
    }
}

void readPlate(char *plate) {                      /* read plate, drop extra characters */
    int c;
    printf("Plate number: ");
    scanf("%15s", plate);
    while ((c = getchar()) != '\n' && c != EOF) ;
}

Record *findRecord(const char *plate) {            /* linear search in list */
    for (Record *p = head; p; p = p->next)
        if (p->active && strcmp(p->plate, plate) == 0) return p;
    return NULL;
}

int allocSlot(int type) {                          /* trucks use far end, others nearest */
    if (type == 2) {
        for (int i = SLOTS - 1; i >= 0; i--) if (!slots[i]) return i;
    } else {
        for (int i = 0; i < SLOTS; i++) if (!slots[i]) return i;
    }
    return -1;
}

void enqueue(Record *r) { waitQ[(qFront + qCount++) % QSIZE] = r; }

Record *dequeue(void) {
    Record *r = waitQ[qFront];
    qFront = (qFront + 1) % QSIZE;
    qCount--;
    return r;
}

/* ---------- main modules ---------- */
void registerVehicle(void) {
    char plate[16];
    readPlate(plate);
    if (findRecord(plate)) { printf("  Vehicle already inside the system.\n"); return; }

    int type = readInt("Type (0=Bike 1=Car 2=Truck): ", 0, 2);
    int hour = readInt("Entry hour (0-23): ", 0, 23);

    int s = allocSlot(type);
    if (s < 0 && qCount == QSIZE) { printf("  Parking and waiting queue are full.\n"); return; }

    Record *r = (Record *)malloc(sizeof(Record));
    strcpy(r->plate, plate);
    r->type = type; r->inHour = hour; r->outHour = hour;
    r->fee = 0; r->active = 1; r->slot = s;
    r->next = head; head = r;                      /* insert at front of list */
    today[hour]++;

    if (s >= 0) { slots[s] = r; printf("  %s parked at slot %d.\n", plate, s + 1); }
    else        { enqueue(r);   printf("  Lot full. %s is waiting (queue position %d).\n", plate, qCount); }
}

void exitVehicle(void) {
    char plate[16];
    readPlate(plate);
    Record *r = findRecord(plate);
    if (!r)           { printf("  Vehicle not found.\n"); return; }
    if (r->slot < 0)  { printf("  Vehicle is still waiting for a slot.\n"); return; }

    int out = readInt("Exit hour (0-23): ", r->inHour, 23);
    int hours = out - r->inHour;
    if (hours == 0) hours = 1;                     /* minimum one hour billed */
    r->outHour = out;
    r->fee = hours * rate[r->type];
    r->active = 0;
    slots[r->slot] = NULL;
    printf("  Bill for %s (%s): %d hr x Rs %d = Rs %d\n",
           r->plate, typeName[r->type], hours, rate[r->type], r->fee);

    int freed = r->slot;
    r->slot = -1;
    if (qCount > 0) {                              /* serve the first waiting vehicle */
        Record *w = dequeue();
        int s = allocSlot(w->type);
        if (s < 0) s = freed;
        w->slot = s; w->inHour = out; slots[s] = w;
        printf("  Waiting vehicle %s moved to slot %d.\n", w->plate, s + 1);
    }
}

void searchVehicle(void) {
    char plate[16];
    readPlate(plate);
    Record *r = findRecord(plate);
    if (!r) { printf("  Not currently in the parking.\n"); return; }
    if (r->slot >= 0) printf("  %s (%s) is at slot %d, entered at %02d:00.\n",
                             r->plate, typeName[r->type], r->slot + 1, r->inHour);
    else printf("  %s is waiting in the queue.\n", r->plate);
}

void showSlots(void) {
    int used = 0;
    printf("\n  Slot  Status\n");
    for (int i = 0; i < SLOTS; i++) {
        if (slots[i]) { used++; printf("  %-5d %s (%s)\n", i + 1, slots[i]->plate, typeName[slots[i]->type]); }
        else printf("  %-5d free\n", i + 1);
    }
    printf("  Occupied: %d/%d   Waiting: %d\n", used, SLOTS, qCount);
}

void statistics(void) {
    int n = 0, revenue = 0, count[3] = {0, 0, 0};
    for (Record *p = head; p; p = p->next) if (!p->active) n++;
    if (n == 0) { printf("  No completed visits yet.\n"); return; }

    Record **a = (Record **)malloc(n * sizeof(Record *));
    int k = 0;
    for (Record *p = head; p; p = p->next)
        if (!p->active) { a[k++] = p; revenue += p->fee; count[p->type]++; }

    for (int i = 1; i < n; i++) {                  /* insertion sort: highest fee first */
        Record *key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j]->fee < key->fee) { a[j + 1] = a[j]; j--; }
        a[j + 1] = key;
    }
    printf("\n  Completed visits (highest fee first)\n");
    for (int i = 0; i < n; i++)
        printf("  %-10s %-6s %02d:00-%02d:00  Rs %d\n", a[i]->plate,
               typeName[a[i]->type], a[i]->inHour, a[i]->outHour, a[i]->fee);
    printf("  Bikes %d | Cars %d | Trucks %d | Revenue Rs %d\n", count[0], count[1], count[2], revenue);
    free(a);
}

void predictDemand(void) {                         /* weighted moving average, newest day weighs most */
    int hour = readInt("Predict demand for hour (0-23): ", 0, 23);
    double p = (3.0 * hist[0][hour] + 2.0 * hist[1][hour] + 1.0 * hist[2][hour]) / 6.0;
    double load = p * 100.0 / SLOTS;
    const char *level = load >= 70 ? "HIGH" : load >= 40 ? "MEDIUM" : "LOW";
    printf("  Expected arrivals at %02d:00 = %.1f (%.0f%% of capacity) -> %s demand\n", hour, p, load, level);

    int best = 0;                                  /* search busiest hour */
    double bestVal = -1;
    for (int h = 0; h < 24; h++) {
        double v = (3.0 * hist[0][h] + 2.0 * hist[1][h] + hist[2][h]) / 6.0;
        if (v > bestVal) { bestVal = v; best = h; }
    }
    printf("  Predicted peak hour of the day: %02d:00 (%.1f arrivals)\n", best, bestVal);
}

int main(void) {
    for (int d = 0; d < DAYS; d++)                 /* seed past-day history */
        for (int h = 0; h < 24; h++) hist[d][h] = pattern[h] + (d == 1) - (d == 2 && pattern[h] > 0);

    int ch;
    do {
        printf("\n=== SMART PARKING SYSTEM ===\n1.Register & Park  2.Exit & Bill  3.Search Vehicle\n"
               "4.Slot Status  5.Statistics  6.Predict Demand  0.Quit\n");
        ch = readInt("Choice: ", 0, 6);
        switch (ch) {
            case 1: registerVehicle(); break;
            case 2: exitVehicle();     break;
            case 3: searchVehicle();   break;
            case 4: showSlots();       break;
            case 5: statistics();      break;
            case 6: predictDemand();   break;
        }
    } while (ch != 0);

    while (head) { Record *t = head; head = head->next; free(t); }
    printf("Goodbye.\n");
    return 0;
}
