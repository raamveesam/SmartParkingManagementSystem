#include "parking_system.h"

void display_parking_statistics(const ParkingSystem* sys) {
    if (!sys) return;

    int occupiedCount = 0;
    int vacantCount = 0;
    int count2W = 0, count4W = 0, countHeavy = 0;

    for (int i = 0; i < sys->totalSlots; i++) {
        if (sys->slots[i].status == SLOT_OCCUPIED) {
            occupiedCount++;
            Vehicle* v = search_registered_vehicle((ParkingSystem*)sys, sys->slots[i].occupiedPlate);
            VehicleType vt = v ? v->type : sys->slots[i].supportedType;
            if (vt == TYPE_TWO_WHEELER) count2W++;
            else if (vt == TYPE_FOUR_WHEELER) count4W++;
            else countHeavy++;
        } else if (sys->slots[i].status == SLOT_VACANT) {
            vacantCount++;
        }
    }

    double occupancyRate = (sys->totalSlots > 0) ? ((double)occupiedCount / sys->totalSlots) * 100.0 : 0.0;

    /* Compute average duration from completed transactions */
    double totalHours = 0.0;
    TransactionNode* curr = sys->historyHead;
    while (curr != NULL) {
        totalHours += curr->data.durationHours;
        curr = curr->next;
    }
    double avgDuration = (sys->totalTransactions > 0) ? (totalHours / sys->totalTransactions) : 0.0;
    double avgTicketRev = (sys->totalTransactions > 0) ? (sys->totalRevenueCollected / sys->totalTransactions) : 0.0;

    printf("\n");
    printf("=========================================================================\n");
    printf("                  PARKING FACILITY OPERATIONAL METRICS                   \n");
    printf("=========================================================================\n");
    printf(" Total Parking Capacity        : %d slots (%d floors, %d slots/floor)\n",
           sys->totalSlots, TOTAL_FLOORS, SLOTS_PER_FLOOR);
    printf(" Currently Occupied Slots      : %d (%.1f%% occupancy)\n", occupiedCount, occupancyRate);
    printf(" Currently Vacant Slots        : %d\n", vacantCount);
    printf(" Vehicles Waiting in Queue     : %d\n", sys->waitingQueue.count);
    printf("-------------------------------------------------------------------------\n");
    printf(" Occupancy Breakdown by Type:\n");
    printf("   - Two-Wheelers              : %d parked\n", count2W);
    printf("   - Four-Wheelers             : %d parked\n", count4W);
    printf("   - Heavy Vehicles            : %d parked\n", countHeavy);
    printf("-------------------------------------------------------------------------\n");
    printf(" Financial & Throughput Summary:\n");
    printf("   - Completed Transactions    : %d\n", sys->totalTransactions);
    printf("   - Total Revenue Collected   : $%.2f\n", sys->totalRevenueCollected);
    printf("   - Average Revenue / Ticket  : $%.2f\n", avgTicketRev);
    printf("   - Average Parking Duration  : %.2f hours\n", avgDuration);
    printf("=========================================================================\n\n");
}
