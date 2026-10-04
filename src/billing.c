#include "parking_system.h"

double get_hourly_rate(VehicleType type) {
    switch (type) {
        case TYPE_TWO_WHEELER:
            return RATE_TWO_WHEELER;
        case TYPE_FOUR_WHEELER:
            return RATE_FOUR_WHEELER;
        case TYPE_HEAVY_VEHICLE:
            return RATE_HEAVY_VEHICLE;
        default:
            return RATE_FOUR_WHEELER;
    }
}

double calculate_parking_charges(VehicleType type, double durationHours, double surgeFactor) {
    if (durationHours < MIN_BILLING_HOURS) {
        durationHours = MIN_BILLING_HOURS;
    }
    double baseRate = get_hourly_rate(type);
    double total = baseRate * durationHours * surgeFactor;
    return round(total * 100.0) / 100.0; /* round to 2 decimals */
}

void print_bill_receipt(const Transaction* tx) {
    if (!tx) return;

    char entryBuf[TIME_STR_LEN];
    char exitBuf[TIME_STR_LEN];
    format_timestamp(tx->entryTime, entryBuf, sizeof(entryBuf));
    format_timestamp(tx->exitTime, exitBuf, sizeof(exitBuf));

    printf("\n");
    printf("*************************************************************\n");
    printf("*               SMART PARKING RECEIPT & INVOICE             *\n");
    printf("*************************************************************\n");
    printf(" Ticket Number     : #%04d\n", tx->ticketId);
    printf(" Vehicle Plate     : %s\n", tx->licensePlate);
    printf(" Vehicle Type      : %s\n", vehicle_type_to_string(tx->vehicleType));
    printf(" Allocated Slot    : Floor %d, Slot #%02d\n", tx->floor, tx->slotId);
    printf("-------------------------------------------------------------\n");
    printf(" Entry Time        : %s\n", entryBuf);
    printf(" Exit Time         : %s\n", exitBuf);
    printf(" Duration (Hours)  : %.2f hrs\n", tx->durationHours);
    printf(" Base Hourly Rate  : $%.2f / hr\n", tx->baseRate);
    printf(" AI Dynamic Surge  : %.2fx\n", tx->surgeFactor);
    printf("-------------------------------------------------------------\n");
    printf(" TOTAL DUE / PAID  : $%.2f\n", tx->totalCharge);
    printf("*************************************************************\n");
    printf("              Thank you for parking with us!                 \n");
    printf("*************************************************************\n\n");
}

void add_transaction_to_history(ParkingSystem* sys, Transaction tx) {
    if (!sys) return;

    TransactionNode* newNode = (TransactionNode*)malloc(sizeof(TransactionNode));
    if (!newNode) {
        fprintf(stderr, "Error: Memory allocation failed for transaction history.\n");
        return;
    }

    newNode->data = tx;
    newNode->next = NULL;

    /* Append to transaction linked list */
    if (sys->historyHead == NULL) {
        sys->historyHead = newNode;
    } else {
        TransactionNode* curr = sys->historyHead;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = newNode;
    }

    sys->totalTransactions++;
    sys->totalRevenueCollected += tx.totalCharge;

    /* Feed AI hourly arrival counter */
    struct tm* entryTm = localtime(&(tx.entryTime));
    if (entryTm) {
        int h = entryTm->tm_hour;
        if (h >= 0 && h < 24) {
            sys->aiModel.hourlyHistoricalCount[h] += 1.0;
        }
    }
}

void display_transaction_history(const ParkingSystem* sys) {
    if (!sys) return;

    printf("\n=========================================================================================================\n");
    printf("                                  BILLING & TRANSACTION HISTORY (LINKED LIST)                            \n");
    printf("=========================================================================================================\n");
    printf("%-8s | %-12s | %-12s | %-6s | %-19s | %-7s | %-7s | %-9s\n",
           "Ticket", "Plate", "Type", "Slot", "Exit Time", "Hours", "Surge", "Total ($)");
    printf("---------------------------------------------------------------------------------------------------------\n");

    TransactionNode* curr = sys->historyHead;
    int count = 0;
    char exitBuf[TIME_STR_LEN];

    while (curr != NULL) {
        format_timestamp(curr->data.exitTime, exitBuf, sizeof(exitBuf));
        const char* shortType = (curr->data.vehicleType == TYPE_TWO_WHEELER) ? "2-Wheeler" :
                                (curr->data.vehicleType == TYPE_FOUR_WHEELER) ? "4-Wheeler" : "Heavy";
        printf("#%06d | %-12s | %-12s | #%-4d | %-19s | %-7.2f | %-6.2fx | $%-8.2f\n",
               curr->data.ticketId,
               curr->data.licensePlate,
               shortType,
               curr->data.slotId,
               exitBuf,
               curr->data.durationHours,
               curr->data.surgeFactor,
               curr->data.totalCharge);
        curr = curr->next;
        count++;
    }

    if (count == 0) {
        printf("  [No completed parking sessions found]\n");
    }
    printf("---------------------------------------------------------------------------------------------------------\n");
    printf("Total Transactions: %d | Total Revenue Collected: $%.2f\n\n", count, sys->totalRevenueCollected);
}

void free_transaction_history(ParkingSystem* sys) {
    if (!sys) return;
    TransactionNode* curr = sys->historyHead;
    while (curr != NULL) {
        TransactionNode* temp = curr;
        curr = curr->next;
        free(temp);
    }
    sys->historyHead = NULL;
    sys->totalTransactions = 0;
    sys->totalRevenueCollected = 0.0;
}
