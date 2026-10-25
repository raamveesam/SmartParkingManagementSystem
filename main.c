/*
 * ============================================================
 *  BayTrack: Multi-Level Parking Management System
 *  Language: C
 *
 *  Core DSA:
 *   [1] Static Array   -> bay_grid[] holds every physical parking bay
 *   [2] Linked List    -> ExitLog chain builds the billing audit trail
 *   [3] FIFO Queue     -> HoldLine queues vehicles when the lot is full
 *   [4] Linear Search  -> scan bay_grid[] to locate a parked vehicle
 *   [5] Insertion Sort -> rank completed sessions by fee collected
 *   [6] Binary Search  -> ticket-id lookup on sorted session list
 *   [7] AI Heuristic   -> Gaussian PeakScore forecasts hourly demand
 *
 *  Tariff (Indian Rupees):
 *   Two-Wheeler  : Rs.20/hr  (6 dedicated bays on Ground floor)
 *   Four-Wheeler : Rs.50/hr  (14 bays across Ground + First floor)
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* ---- Configurable constants ---- */
#define BAY_COUNT      20
#define BAYS_PER_LEVEL 10
#define TW_BAY_LIMIT    6    /* ground-floor bays reserved for two-wheelers  */
#define RATE_TW        20.0  /* Rs. per hour, two-wheelers                   */
#define RATE_FW        50.0  /* Rs. per hour, four-wheelers                  */
#define MIN_BILLABLE    1.0  /* minimum chargeable hours                     */
#define TICKET_START  5001   /* first pass number issued                     */

/* ---- Vehicle category codes ---- */
#define CAT_TW  1
#define CAT_FW  2

/* ---- Bay occupancy flags ---- */
#define BAY_FREE 0
#define BAY_BUSY 1

/* ============================================================
 *  DATA STRUCTURES
 * ============================================================ */

/*
 * ParkBay — one physical slot in the static bay_grid[] array.
 * When a vehicle parks, its number is stored in 'tag'.
 */
typedef struct {
    int   bayNum;          /* bay identifier 1..BAY_COUNT           */
    int   levelNum;        /* 0 = Ground, 1 = First floor           */
    int   category;        /* CAT_TW or CAT_FW                      */
    int   status;          /* BAY_FREE or BAY_BUSY                  */
    char  tag[18];         /* vehicle number currently parked here  */
    int   passNo;          /* entry pass (ticket) number            */
    long  arrivalEpoch;    /* unix timestamp of vehicle arrival     */
} ParkBay;

/*
 * CarInfo — minimal descriptor kept inside the HoldLine queue.
 */
typedef struct {
    char tag[18];
    char driverName[36];
    int  category;
} CarInfo;

/*
 * HoldNode — single node of the FIFO waiting queue.
 * 'next' points toward the rear (later arrivals).
 */
typedef struct HoldNode {
    CarInfo         car;
    struct HoldNode *next;
} HoldNode;

/*
 * HoldLine — the FIFO queue wrapper.
 * head -> served first (earliest arrival)
 * tail -> most recent addition
 */
typedef struct {
    HoldNode *head;
    HoldNode *tail;
    int       size;
} HoldLine;

/*
 * ExitLog — one billing record in the singly linked list.
 * New records are prepended at 'ledger' (head pointer).
 */
typedef struct ExitLog {
    int   passNo;
    char  tag[18];
    int   category;
    int   bayNum;
    long  inEpoch;
    long  outEpoch;
    float hoursParked;
    float peakMult;
    float feePaid;
    struct ExitLog *prev;   /* chain toward older records */
} ExitLog;

/* ============================================================
 *  GLOBAL STATE
 * ============================================================ */
static ParkBay  bay_grid[BAY_COUNT];  /* ARRAY  — the physical parking map  */
static HoldLine hold_line;            /* QUEUE  — overflow waiting vehicles  */
static ExitLog *ledger    = NULL;     /* LIST   — billing audit trail        */
static int      pass_ctr  = TICKET_START;
static float    vault     = 0.0f;    /* cumulative revenue collected (Rs.)  */
static int      closed_n  = 0;       /* number of completed parking sessions */

/* ============================================================
 *  HELPER UTILITIES
 * ============================================================ */

/*
 * same_tag() — case-insensitive string comparison for vehicle
 * registration numbers.  Written from scratch without stricmp
 * to stay ANSI-portable across Windows MinGW and Linux GCC.
 */
static int same_tag(const char *a, const char *b)
{
    while (*a && *b) {
        char x = (*a >= 'a' && *a <= 'z') ? (char)(*a - 32) : *a;
        char y = (*b >= 'a' && *b <= 'z') ? (char)(*b - 32) : *b;
        if (x != y) return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

/* epoch_to_str() — converts unix timestamp to "DD-Mon HH:MM" format */
static void epoch_to_str(long ep, char *buf, int sz)
{
    time_t t = (time_t)ep;
    struct tm *info = localtime(&t);
    if (info) strftime(buf, sz, "%d-%b %H:%M", info);
    else      snprintf(buf, sz, "N/A");
}

static const char *cat_label(int c)
{
    return (c == CAT_TW) ? "Two-Wheeler" : "Four-Wheeler";
}

static float hourly_rate(int c)
{
    return (c == CAT_TW) ? (float)RATE_TW : (float)RATE_FW;
}

/* ============================================================
 *  BAY GRID INITIALISATION  (Array)
 *
 *  Ground floor (level 0): bays 1-10
 *    bays 1-6  -> two-wheeler zone
 *    bays 7-10 -> four-wheeler zone
 *  First floor  (level 1): bays 11-20
 *    all bays  -> four-wheeler zone
 * ============================================================ */

void setup_bay_grid(void)
{
    int k;
    for (k = 0; k < BAY_COUNT; k++) {
        bay_grid[k].bayNum       = k + 1;
        bay_grid[k].levelNum     = k / BAYS_PER_LEVEL;
        bay_grid[k].status       = BAY_FREE;
        bay_grid[k].tag[0]       = '\0';
        bay_grid[k].passNo       = 0;
        bay_grid[k].arrivalEpoch = 0;
        bay_grid[k].category     = (k < TW_BAY_LIMIT) ? CAT_TW : CAT_FW;
    }
    hold_line.head = NULL;
    hold_line.tail = NULL;
    hold_line.size = 0;
}

/* ============================================================
 *  QUEUE OPERATIONS  (FIFO)
 * ============================================================ */

/* push_hold() appends a car to the REAR of the waiting queue */
static void push_hold(CarInfo c)
{
    HoldNode *nd = (HoldNode *)malloc(sizeof(HoldNode));
    if (!nd) { fprintf(stderr, "Memory error in queue.\n"); return; }
    nd->car  = c;
    nd->next = NULL;
    if (hold_line.tail == NULL) {
        hold_line.head = nd;
        hold_line.tail = nd;
    } else {
        hold_line.tail->next = nd;
        hold_line.tail       = nd;
    }
    hold_line.size++;
    printf("  [QUEUE] '%s' entered hold-line at position #%d.\n",
           c.tag, hold_line.size);
}

/* pop_hold() removes and returns the FRONT car from the queue */
static int pop_hold(CarInfo *dest)
{
    if (!hold_line.head) return 0;
    HoldNode *gone = hold_line.head;
    if (dest) *dest = gone->car;
    hold_line.head = gone->next;
    if (!hold_line.head) hold_line.tail = NULL;
    free(gone);
    hold_line.size--;
    return 1;
}

/* ============================================================
 *  AI MODULE — Gaussian PeakScore Demand Forecaster
 *
 *  peak_score(h) models two urban rush windows using a sum of
 *  Gaussian bell curves:
 *    Morning rush  : centred at 09:00 (sigma ≈ 1.4 hr)
 *    Evening rush  : centred at 18:30 (sigma ≈ 1.3 hr)
 *  A flat daytime baseline handles midday occupancy.
 *  Output is clamped to [0.0, 1.0].
 *
 *  pricing_mult() maps the score to a tariff multiplier —
 *  high demand raises the rate, off-peak gives a small rebate.
 * ============================================================ */

static float peak_score(int hour)
{
    double h   = (double)hour;
    double mng = 0.85 * exp(-0.5 * ((h - 9.0)  / 1.4) * ((h - 9.0)  / 1.4));
    double evn = 0.94 * exp(-0.5 * ((h - 18.5) / 1.3) * ((h - 18.5) / 1.3));
    double day = (h >= 10.0 && h <= 17.0) ? 0.62 : 0.0;
    double sc  = mng + evn + day;
    if (sc > 1.0) sc = 1.0;
    if (sc < 0.0) sc = 0.0;
    return (float)sc;
}

static float pricing_mult(float sc)
{
    if (sc >= 0.80f) return 1.60f;
    if (sc >= 0.55f) return 1.25f;
    if (sc <= 0.20f) return 0.80f;
    return 1.00f;
}

/* show_forecast() — prints a 24-hour demand bar chart */
void show_forecast(void)
{
    int h, b;
    printf("\n+------+-------+------------------------+--------------------+\n");
    printf("| Hour | Score | Bar (each | = 5%%)      | Tariff Tier        |\n");
    printf("+------+-------+------------------------+--------------------+\n");
    for (h = 0; h < 24; h++) {
        float sc   = peak_score(h);
        float mult = pricing_mult(sc);
        int   bars = (int)(sc * 20.0f);
        char  bar[22];
        for (b = 0; b < bars; b++) bar[b] = '|';
        bar[b] = '\0';
        const char *tier;
        if      (mult >= 1.6f)  tier = "PEAK SURGE  x1.60";
        else if (mult >= 1.25f) tier = "MODERATE    x1.25";
        else if (mult < 1.0f)   tier = "OFF-PEAK    x0.80";
        else                    tier = "NORMAL      x1.00";
        printf("| %02d:00| %.2f  | %-22s | %-18s |\n",
               h, sc, bar, tier);
    }
    printf("+------+-------+------------------------+--------------------+\n\n");
}

/* ============================================================
 *  BAY ASSIGNMENT  (Array linear scan)
 * ============================================================ */

/* assign_bay() scans left-to-right for the first free matching bay */
static int assign_bay(int cat, const char *tag, int passNo, long epoch)
{
    int k;
    for (k = 0; k < BAY_COUNT; k++) {
        if (bay_grid[k].status == BAY_FREE && bay_grid[k].category == cat) {
            bay_grid[k].status       = BAY_BUSY;
            bay_grid[k].passNo       = passNo;
            bay_grid[k].arrivalEpoch = epoch;
            strncpy(bay_grid[k].tag, tag, sizeof(bay_grid[k].tag) - 1);
            bay_grid[k].tag[sizeof(bay_grid[k].tag) - 1] = '\0';
            return bay_grid[k].bayNum;
        }
    }
    return -1;   /* no bay of this category is free */
}

/* release_bay() clears the bay that holds a given vehicle tag */
static int release_bay(const char *tag)
{
    int k;
    for (k = 0; k < BAY_COUNT; k++) {
        if (bay_grid[k].status == BAY_BUSY && same_tag(bay_grid[k].tag, tag)) {
            bay_grid[k].status       = BAY_FREE;
            bay_grid[k].tag[0]       = '\0';
            bay_grid[k].passNo       = 0;
            bay_grid[k].arrivalEpoch = 0;
            return k;
        }
    }
    return -1;
}

/* ============================================================
 *  VEHICLE ARRIVAL  (Check-In)
 * ============================================================ */

void vehicle_arrive(const char *tag, const char *driver,
                    int cat, long customEpoch)
{
    int k;
    /* Reject duplicate: vehicle already occupies a bay */
    for (k = 0; k < BAY_COUNT; k++) {
        if (bay_grid[k].status == BAY_BUSY && same_tag(bay_grid[k].tag, tag)) {
            printf("  [WARN] %s is already in Bay #%d.\n", tag, bay_grid[k].bayNum);
            return;
        }
    }

    long arrEp = (customEpoch > 0) ? customEpoch : (long)time(NULL);
    int  pass  = pass_ctr++;
    int  bay   = assign_bay(cat, tag, pass, arrEp);

    if (bay != -1) {
        char ts[24];
        epoch_to_str(arrEp, ts, sizeof(ts));
        printf("\n  +-- ENTRY PASS GENERATED ---------------------------------+\n");
        printf("  | Pass   : #%d\n", pass);
        printf("  | Vehicle: %-14s   Driver : %-20s |\n", tag, driver);
        printf("  | Type   : %-14s   Bay #%02d  Level %d               |\n",
               cat_label(cat), bay, bay_grid[bay-1].levelNum);
        printf("  | Arrived: %-28s                     |\n", ts);
        printf("  | Rate   : Rs.%.0f/hr                                    |\n",
               hourly_rate(cat));
        printf("  +---------------------------------------------------------+\n");
    } else {
        /* Lot is full for this category — add to FIFO hold queue */
        CarInfo ci;
        strncpy(ci.tag, tag, sizeof(ci.tag)-1);
        ci.tag[sizeof(ci.tag)-1] = '\0';
        strncpy(ci.driverName, driver, sizeof(ci.driverName)-1);
        ci.driverName[sizeof(ci.driverName)-1] = '\0';
        ci.category = cat;
        push_hold(ci);
    }
}

/* ============================================================
 *  VEHICLE DEPARTURE  (Check-Out + Billing)
 * ============================================================ */

void vehicle_depart(const char *tag, long customEpoch)
{
    int k;
    int foundIdx = -1;

    /* Linear search across the bay_grid array */
    for (k = 0; k < BAY_COUNT; k++) {
        if (bay_grid[k].status == BAY_BUSY && same_tag(bay_grid[k].tag, tag)) {
            foundIdx = k;
            break;
        }
    }
    if (foundIdx == -1) {
        printf("  [WARN] '%s' is not in any active bay.\n", tag);
        return;
    }

    ParkBay snap    = bay_grid[foundIdx];
    long    outEp   = (customEpoch > 0) ? customEpoch : (long)time(NULL);
    float   rawHrs  = (float)difftime((time_t)outEp, (time_t)snap.arrivalEpoch) / 3600.0f;
    float   billHrs = (rawHrs < (float)MIN_BILLABLE) ? (float)MIN_BILLABLE : rawHrs;

    /* Query AI model for the departure hour */
    struct tm *dptm = localtime((time_t *)&outEp);
    int    hr       = dptm ? dptm->tm_hour : 12;
    float  sc       = peak_score(hr);
    float  mult     = pricing_mult(sc);
    float  rate     = hourly_rate(snap.category);
    float  fee      = rate * billHrs * mult;

    /* Prepend a new ExitLog node to the billing linked list */
    ExitLog *rec = (ExitLog *)malloc(sizeof(ExitLog));
    if (!rec) { fprintf(stderr, "Memory error — billing.\n"); return; }
    rec->passNo     = snap.passNo;
    rec->category   = snap.category;
    rec->bayNum     = snap.bayNum;
    rec->inEpoch    = snap.arrivalEpoch;
    rec->outEpoch   = outEp;
    rec->hoursParked= billHrs;
    rec->peakMult   = mult;
    rec->feePaid    = fee;
    strncpy(rec->tag, snap.tag, sizeof(rec->tag)-1);
    rec->tag[sizeof(rec->tag)-1] = '\0';
    rec->prev = ledger;
    ledger    = rec;

    vault    += fee;
    closed_n++;

    /* Print payment receipt */
    char arrStr[24], dptStr[24];
    epoch_to_str(snap.arrivalEpoch, arrStr, sizeof(arrStr));
    epoch_to_str(outEp, dptStr, sizeof(dptStr));

    printf("\n  +============== PAYMENT RECEIPT ==========================+\n");
    printf("  | Pass #%-6d   Vehicle : %-26s   |\n", rec->passNo, rec->tag);
    printf("  | Type  : %-38s   |\n", cat_label(rec->category));
    printf("  | Bay   : #%02d   Arrived : %-28s|\n", rec->bayNum, arrStr);
    printf("  | Depart: %-38s   |\n", dptStr);
    printf("  | Hours : %.1f hrs   Rate : Rs.%.0f/hr                   |\n",
           billHrs, rate);
    printf("  | PeakScore: %.2f   Price-Mult: x%.2f                    |\n",
           sc, mult);
    printf("  |---------------------------------------------------------|\n");
    printf("  | AMOUNT DUE : Rs. %.2f                                |\n", fee);
    printf("  +=========================================================+\n");

    /* Free the bay in the array */
    release_bay(tag);
    printf("  [BAY] Bay #%02d is now free.\n", snap.bayNum);

    /* Auto-assign freed bay to the first matching car in the queue */
    if (hold_line.size > 0 && hold_line.head->car.category == snap.category) {
        CarInfo waiting;
        pop_hold(&waiting);
        int nb = assign_bay(waiting.category, waiting.tag, pass_ctr++, outEp);
        if (nb != -1)
            printf("  [AUTO] '%s' moved from queue to Bay #%02d.\n\n", waiting.tag, nb);
    }
}

/* ============================================================
 *  SEARCHING — Linear Scan
 * ============================================================ */

int find_vehicle(const char *tag)
{
    int k;
    for (k = 0; k < BAY_COUNT; k++)
        if (bay_grid[k].status == BAY_BUSY && same_tag(bay_grid[k].tag, tag))
            return bay_grid[k].bayNum;
    return -1;
}

/* ============================================================
 *  SORTING — Insertion Sort on billing records (descending fee)
 *
 *  Why Insertion Sort?
 *    The billing ledger is a linked list.  We extract node pointers
 *    into a small helper array and sort in-place.  Insertion sort
 *    suits small-to-medium datasets (typical for a parking lot)
 *    and produces stable, predictable output — useful when fees
 *    are equal (ties are kept in arrival order).
 * ============================================================ */

void show_sorted_sessions(void)
{
    int i, j, n = 0;
    ExitLog *cur = ledger;

    if (!cur) { printf("  No completed sessions yet.\n"); return; }

    /* Count nodes in the list */
    while (cur) { n++; cur = cur->prev; }

    ExitLog **arr = (ExitLog **)malloc((size_t)n * sizeof(ExitLog *));
    if (!arr) return;

    /* Load pointers into the helper array */
    cur = ledger;
    for (i = 0; i < n; i++, cur = cur->prev) arr[i] = cur;

    /* Insertion sort — descending by feePaid */
    for (i = 1; i < n; i++) {
        ExitLog *key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j]->feePaid < key->feePaid) {
            arr[j+1] = arr[j];
            j--;
        }
        arr[j+1] = key;
    }

    printf("\n  +-- Sessions Ranked by Fee Collected (High -> Low) ------+\n");
    printf("  | Rank | Pass   | Vehicle        | Hrs   | Fee (Rs)  |\n");
    printf("  |------+--------+----------------+-------+-----------|\n");
    for (i = 0; i < n; i++) {
        printf("  | %4d | #%-5d | %-14s | %5.1f | %9.2f |\n",
               i+1, arr[i]->passNo, arr[i]->tag,
               arr[i]->hoursParked, arr[i]->feePaid);
    }
    printf("  +--------------------------------------------------------+\n\n");
    free(arr);
}

/* ============================================================
 *  BINARY SEARCH — find a session by pass number
 *  (array must be sorted by passNo ascending before calling)
 * ============================================================ */

static int bsearch_pass(ExitLog **arr, int n, int target)
{
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (arr[mid]->passNo == target) return mid;
        if (arr[mid]->passNo  < target) lo = mid + 1;
        else                            hi = mid - 1;
    }
    return -1;
}

void lookup_pass(int target)
{
    int i, j, n = 0;
    ExitLog *cur = ledger;

    if (!cur) { printf("  No records exist yet.\n"); return; }
    while (cur) { n++; cur = cur->prev; }

    ExitLog **arr = (ExitLog **)malloc((size_t)n * sizeof(ExitLog *));
    if (!arr) return;
    cur = ledger;
    for (i = 0; i < n; i++, cur = cur->prev) arr[i] = cur;

    /* Sort by passNo ascending so binary search can work correctly */
    for (i = 1; i < n; i++) {
        ExitLog *key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j]->passNo > key->passNo) {
            arr[j+1] = arr[j]; j--;
        }
        arr[j+1] = key;
    }

    int pos = bsearch_pass(arr, n, target);
    if (pos >= 0)
        printf("  [FOUND] Pass #%d -> %s | Bay #%d | Rs.%.2f\n",
               target, arr[pos]->tag, arr[pos]->bayNum, arr[pos]->feePaid);
    else
        printf("  [NOT FOUND] Pass #%d has no matching record.\n", target);

    free(arr);
}

/* ============================================================
 *  DISPLAY UTILITIES
 * ============================================================ */

void show_bay_layout(void)
{
    int lv, k;
    printf("\n  +-- Bay Layout Map ----------------------------------------+\n");
    for (lv = 0; lv < 2; lv++) {
        printf("  | Level %d: ", lv);
        for (k = 0; k < BAY_COUNT; k++) {
            if (bay_grid[k].levelNum == lv)
                printf("[%02d:%s] ",
                       bay_grid[k].bayNum,
                       (bay_grid[k].status == BAY_FREE) ? "--" : "XX");
        }
        printf("|\n");
    }
    printf("  | Key: [--] = Vacant   [XX] = Occupied                   |\n");
    printf("  +---------------------------------------------------------+\n\n");
}

void show_hold_queue(void)
{
    HoldNode *nd = hold_line.head;
    int pos = 1;
    printf("\n  +-- Hold-Line (FIFO Queue) --------------------------------+\n");
    if (!nd) {
        printf("  | Queue is currently empty.                               |\n");
    } else {
        while (nd) {
            printf("  | #%d  %-16s %-14s                   |\n",
                   pos, nd->car.tag, cat_label(nd->car.category));
            nd = nd->next; pos++;
        }
    }
    printf("  +---------------------------------------------------------+\n\n");
}

void show_summary(void)
{
    int k, occ = 0;
    for (k = 0; k < BAY_COUNT; k++)
        if (bay_grid[k].status == BAY_BUSY) occ++;

    printf("\n  +-- Facility Summary -------------------------------------+\n");
    printf("  | Total Bays      : %d                                    |\n", BAY_COUNT);
    printf("  | Occupied        : %d                                    |\n", occ);
    printf("  | Available       : %d                                    |\n", BAY_COUNT - occ);
    printf("  | Hold Queue Size : %d                                    |\n", hold_line.size);
    printf("  | Sessions Closed : %d                                    |\n", closed_n);
    printf("  | Total Revenue   : Rs. %.2f                           |\n", vault);
    printf("  +---------------------------------------------------------+\n\n");
}

/* ============================================================
 *  AUTOMATED DEMO  (uses Indian vehicle registration examples)
 * ============================================================ */

void run_demo(void)
{
    long now = (long)time(NULL);

    printf("\n======================================================\n");
    printf("   BayTrack — Full-System Demonstration Session       \n");
    printf("======================================================\n\n");

    /* Vehicles arrive at different times */
    vehicle_arrive("KA-05-MJ-7823", "Ramesh Iyer",   CAT_FW, now - 9000);
    vehicle_arrive("TN-09-AB-3341", "Meena Suresh",  CAT_TW, now - 7200);
    vehicle_arrive("MH-14-CD-6610", "Arjun Patil",   CAT_FW, now - 5400);
    vehicle_arrive("AP-28-EF-1120", "Divya Reddy",   CAT_TW, now - 4800);
    vehicle_arrive("DL-01-GH-9902", "Priya Mehta",   CAT_FW, now - 3600);
    vehicle_arrive("KL-07-JK-4456", "Suresh Nair",   CAT_TW, now - 3000);
    vehicle_arrive("TS-11-AA-0001", "Ravi Kumar",    CAT_TW, now - 1800);
    vehicle_arrive("GJ-06-BB-0002", "Neha Shah",     CAT_TW, now - 1200);
    vehicle_arrive("RJ-14-CC-0003", "Anil Joshi",    CAT_TW, now -  900);
    /* All 6 TW bays now full — next TW must queue */
    vehicle_arrive("UP-32-DD-0004", "Sunil Verma",   CAT_TW, now -  300);

    printf("\n--- Bay Layout after initial arrivals ---\n");
    show_bay_layout();
    show_hold_queue();

    /* Search for a parked vehicle using linear scan */
    printf("  [LINEAR SEARCH] Locating 'MH-14-CD-6610'...\n");
    int slot = find_vehicle("MH-14-CD-6610");
    if (slot != -1)
        printf("  [RESULT] Found in Bay #%02d.\n\n", slot);

    /* A TW departs — queued UP-32-DD-0004 should auto-occupy freed bay */
    printf("  [EXIT] TN-09-AB-3341 departing...\n");
    vehicle_depart("TN-09-AB-3341", now);

    /* Another exit to build more billing history */
    printf("  [EXIT] KA-05-MJ-7823 departing...\n");
    vehicle_depart("KA-05-MJ-7823", now);

    /* Show billing sessions ranked by fee — uses Insertion Sort */
    show_sorted_sessions();

    /* Binary-search demo on a known pass number */
    printf("  [BINARY SEARCH] Looking up Pass #%d...\n", TICKET_START);
    lookup_pass(TICKET_START);

    /* AI demand forecast across 24 hours */
    printf("\n  [AI FORECAST] 24-Hour Occupancy & Tariff Schedule:\n");
    show_forecast();

    show_summary();

    printf("======================================================\n");
    printf("                 Demo Complete.                       \n");
    printf("======================================================\n\n");
}

/* ============================================================
 *  INTERACTIVE MAIN MENU
 * ============================================================ */

static void read_line(char *buf, int sz)
{
    if (fgets(buf, sz, stdin))
        buf[strcspn(buf, "\r\n")] = '\0';
}

int main(int argc, char *argv[])
{
    setup_bay_grid();

    /* Headless demo mode: ./baytrack --demo */
    if (argc > 1 && strcmp(argv[1], "--demo") == 0) {
        run_demo();
        return 0;
    }

    char line[64];
    int  choice;

    for (;;) {
        printf("\n+==================[ BayTrack Menu ]==================+\n");
        printf("|  1. Register Arrival    (Check-In)                  |\n");
        printf("|  2. Register Departure  (Check-Out + Bill)          |\n");
        printf("|  3. Find Vehicle Bay    (Linear Search)             |\n");
        printf("|  4. View Bay Layout Map                             |\n");
        printf("|  5. View Hold-Line Queue                            |\n");
        printf("|  6. Ranked Billing Sessions (Insertion Sort)        |\n");
        printf("|  7. Lookup Pass Number  (Binary Search)             |\n");
        printf("|  8. AI Demand Forecast & Tariff Schedule            |\n");
        printf("|  9. Facility Summary & Revenue                      |\n");
        printf("| 10. Run Automated Demo                              |\n");
        printf("|  0. Exit                                            |\n");
        printf("+=====================================================+\n");
        printf("Choice: ");
        read_line(line, sizeof(line));
        if (sscanf(line, "%d", &choice) != 1) continue;
        if (choice == 0) break;

        switch (choice) {
            case 1: {
                char tag[18], drv[36], cb[4];
                int  cat;
                printf("Vehicle Number (e.g. KA-01-AB-1234): "); read_line(tag, sizeof(tag));
                printf("Driver Name: ");                          read_line(drv, sizeof(drv));
                printf("Category (1=Two-Wheeler, 2=Four-Wheeler): "); read_line(cb, sizeof(cb));
                cat = (cb[0] == '1') ? CAT_TW : CAT_FW;
                vehicle_arrive(tag, drv, cat, 0);
                break;
            }
            case 2: {
                char tag[18];
                printf("Vehicle Number to Depart: "); read_line(tag, sizeof(tag));
                vehicle_depart(tag, 0);
                break;
            }
            case 3: {
                char tag[18];
                printf("Vehicle Number: "); read_line(tag, sizeof(tag));
                int b = find_vehicle(tag);
                if (b != -1) printf("  Found in Bay #%02d.\n", b);
                else         printf("  Not currently parked here.\n");
                break;
            }
            case 4:  show_bay_layout();      break;
            case 5:  show_hold_queue();      break;
            case 6:  show_sorted_sessions(); break;
            case 7: {
                char pb[12]; int p;
                printf("Enter Pass Number: "); read_line(pb, sizeof(pb));
                if (sscanf(pb, "%d", &p) == 1) lookup_pass(p);
                break;
            }
            case 8:  show_forecast();  break;
            case 9:  show_summary();   break;
            case 10: run_demo();       break;
            default: printf("  Invalid option.\n"); break;
        }
    }
    printf("Goodbye from BayTrack!\n");
    return 0;
}
