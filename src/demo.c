#include "parking_system.h"

void run_automated_demo(ParkingSystem* sys) {
    printf("\n#########################################################################\n");
    printf("#        STARTING AUTOMATED END-TO-END DEMO SIMULATION                  #\n");
    printf("#########################################################################\n\n");

    /* Step 1: Pre-train the AI model */
    printf("--> Step 1: Initializing & Training AI Demand Model...\n");
    train_ai_demand_model(sys);

    /* Step 2: Show initial state */
    printf("\n--> Step 2: Displaying Initial Empty Facility Layout...\n");
    display_parking_grid_ascii(sys);

    /* Step 3: Vehicle Registrations */
    printf("--> Step 3: Registering Test Vehicles (Linked List)...\n");
    register_vehicle(sys, "KA-01-AB-1234", "Dr. Arvind Rao", "+91-9876543210", TYPE_FOUR_WHEELER);
    register_vehicle(sys, "DL-04-CD-5678", "Ms. Neha Sharma", "+91-9123456789", TYPE_TWO_WHEELER);
    register_vehicle(sys, "MH-12-EF-9012", "Mr. Rajesh Kumar", "+91-9988776655", TYPE_FOUR_WHEELER);
    register_vehicle(sys, "AP-09-GH-3456", "Apex Logistics", "+91-9445566778", TYPE_HEAVY_VEHICLE);
    register_vehicle(sys, "TN-07-JK-7890", "Prof. S. Nathan", "+91-9334455667", TYPE_TWO_WHEELER);

    display_registered_vehicles(sys);

    /* Step 4: Simulate Vehicle Check-Ins (Entry) */
    printf("--> Step 4: Processing Vehicle Entries & Slot Allocation (Array)...\n");
    time_t now = time(NULL);
    process_vehicle_entry(sys, "KA-01-AB-1234", "Dr. Arvind Rao", NULL, TYPE_FOUR_WHEELER, now - 7200); /* 2 hrs ago */
    process_vehicle_entry(sys, "DL-04-CD-5678", "Ms. Neha Sharma", NULL, TYPE_TWO_WHEELER, now - 5400);  /* 1.5 hrs ago */
    process_vehicle_entry(sys, "MH-12-EF-9012", "Mr. Rajesh Kumar", NULL, TYPE_FOUR_WHEELER, now - 3600); /* 1 hr ago */
    process_vehicle_entry(sys, "AP-09-GH-3456", "Apex Logistics", NULL, TYPE_HEAVY_VEHICLE, now - 1800);  /* 0.5 hr ago */

    /* Step 5: Fill remaining heavy vehicle slots to test Queue overflow */
    printf("\n--> Step 5: Simulating Capacity Saturation to Trigger Waiting Queue (FIFO Queue)...\n");
    /* We have 5 heavy slots: 26, 27, 28, 29, 30. One is taken by AP-09-GH-3456. Let's fill the rest. */
    process_vehicle_entry(sys, "HV-TRUCK-01", "Fleet Corp 1", "001", TYPE_HEAVY_VEHICLE, now - 2000);
    process_vehicle_entry(sys, "HV-TRUCK-02", "Fleet Corp 2", "002", TYPE_HEAVY_VEHICLE, now - 1500);
    process_vehicle_entry(sys, "HV-TRUCK-03", "Fleet Corp 3", "003", TYPE_HEAVY_VEHICLE, now - 1000);
    process_vehicle_entry(sys, "HV-TRUCK-04", "Fleet Corp 4", "004", TYPE_HEAVY_VEHICLE, now - 800);

    /* Now Heavy slots are 100% full! Next heavy vehicle must be enqueued into waiting list */
    printf("\n--> Now all Heavy Vehicle slots are full. Dispatching an overflow vehicle:\n");
    process_vehicle_entry(sys, "HV-OVERFLOW-99", "Express Freight", "999", TYPE_HEAVY_VEHICLE, now);

    /* Check waiting queue status */
    display_waiting_queue(&(sys->waitingQueue));

    /* Step 6: Display visual lot state */
    printf("--> Step 6: Updated Parking Grid with Occupied Slots...\n");
    display_parking_grid_ascii(sys);

    /* Step 7: Searching a Vehicle Slot */
    printf("--> Step 7: Searching for Slot of Vehicle 'MH-12-EF-9012' (Linear Search on Array)...\n");
    int foundSlot = find_slot_by_plate(sys, "MH-12-EF-9012");
    if (foundSlot != -1) {
        printf("[Search Result] Vehicle 'MH-12-EF-9012' is parked at Slot #%02d (Floor %d).\n\n",
               foundSlot, sys->slots[foundSlot - 1].floor);
    }

    /* Step 8: Vehicle Exit & Billing */
    printf("--> Step 8: Processing Checkout & Exit for 'KA-01-AB-1234'...\n");
    process_vehicle_exit(sys, "KA-01-AB-1234", now);

    /* Step 9: Heavy Vehicle Exits -> Testing Queue Auto-Allocation */
    printf("--> Step 9: Processing Exit for Heavy Truck 'AP-09-GH-3456' to trigger auto-allocation from Waiting Queue...\n");
    process_vehicle_exit(sys, "AP-09-GH-3456", now);

    /* Verify that HV-OVERFLOW-99 was dequeued and auto-assigned */
    display_waiting_queue(&(sys->waitingQueue));

    /* Step 10: Sorting Transaction Records (DSA Extension) */
    printf("--> Step 10: Demonstrating Sorting & Binary Search on Completed Transactions...\n");

    /* Add another simulated exit to have more records */
    process_vehicle_exit(sys, "DL-04-CD-5678", now);

    int count = 0;
    Transaction** txArray = get_transactions_array(sys, &count);
    if (txArray && count > 0) {
        printf("\n[Sort 1: QuickSort by Total Charge (Descending)]\n");
        quick_sort_transactions_by_amount(txArray, 0, count - 1, false);
        for (int i = 0; i < count; i++) {
            printf("  Rank %d: Ticket #%04d | Plate: %-14s | Charge: $%.2f\n",
                   i + 1, txArray[i]->ticketId, txArray[i]->licensePlate, txArray[i]->totalCharge);
        }

        printf("\n[Sort 2: MergeSort by Entry Time (Ascending)]\n");
        merge_sort_transactions_by_time(txArray, 0, count - 1);
        char tBuf[TIME_STR_LEN];
        for (int i = 0; i < count; i++) {
            format_timestamp(txArray[i]->entryTime, tBuf, sizeof(tBuf));
            printf("  Entry: %s | Ticket #%04d | Plate: %-14s\n",
                   tBuf, txArray[i]->ticketId, txArray[i]->licensePlate);
        }

        /* Binary Search for a known ticket */
        int targetTicket = txArray[0]->ticketId;
        printf("\n[Search 2: Binary Search for Ticket #%04d]...\n", targetTicket);
        /* To binary search by ticketId, sort array by ticketId first or use sorted order */
        /* Quick bubble sort by ticketId for binary search demonstration */
        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (txArray[j]->ticketId > txArray[j + 1]->ticketId) {
                    Transaction* tmp = txArray[j];
                    txArray[j] = txArray[j + 1];
                    txArray[j + 1] = tmp;
                }
            }
        }
        int bsIdx = binary_search_transaction_by_ticket(txArray, count, targetTicket);
        if (bsIdx != -1) {
            printf("  [+] Binary Search FOUND Ticket #%04d at index %d (Vehicle: %s, Paid: $%.2f)\n\n",
                   targetTicket, bsIdx, txArray[bsIdx]->licensePlate, txArray[bsIdx]->totalCharge);
        }

        free(txArray);
    }

    /* Step 11: AI Demand Dashboard */
    printf("--> Step 11: AI Parking Demand Prediction Forecast...\n");
    display_ai_demand_dashboard(sys);

    /* Step 12: Overall Statistics */
    printf("--> Step 12: Overall Parking Statistics...\n");
    display_parking_statistics(sys);

    printf("#########################################################################\n");
    printf("#                    DEMO COMPLETED SUCCESSFULLY                        #\n");
    printf("#########################################################################\n\n");
}
