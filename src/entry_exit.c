#include "parking_system.h"

bool process_vehicle_entry(ParkingSystem* sys, const char* plate, const char* owner, const char* phone, VehicleType type, time_t customEntry) {
    if (!sys || !plate || strlen(plate) == 0) {
        printf("[!] Invalid entry parameters.\n");
        return false;
    }

    /* Check if already currently parked */
    int existingSlot = find_slot_by_plate(sys, plate);
    if (existingSlot != -1) {
        printf("[!] Error: Vehicle '%s' is already parked in Slot #%02d!\n", plate, existingSlot);
        return false;
    }

    /* Auto-register if not already registered */
    Vehicle* regVeh = search_registered_vehicle(sys, plate);
    if (!regVeh) {
        register_vehicle(sys, plate, owner, phone, type);
        regVeh = search_registered_vehicle(sys, plate);
    } else {
        type = regVeh->type; /* Use registered type */
    }

    time_t entryTime = (customEntry != 0) ? customEntry : time(NULL);
    int ticketId = sys->nextTicketId++;

    int assignedSlot = allocate_slot_for_vehicle(sys, type, plate, ticketId, entryTime);

    if (assignedSlot != -1) {
        int floor = (assignedSlot - 1) / SLOTS_PER_FLOOR + 1;
        char timeBuf[TIME_STR_LEN];
        format_timestamp(entryTime, timeBuf, sizeof(timeBuf));

        printf("\n=======================================================\n");
        printf("               PARKING ENTRY TICKET ISSUED             \n");
        printf("=======================================================\n");
        printf(" Ticket ID      : #%04d\n", ticketId);
        printf(" License Plate  : %s\n", plate);
        printf(" Vehicle Type   : %s\n", vehicle_type_to_string(type));
        printf(" Owner Name     : %s\n", regVeh ? regVeh->ownerName : "Guest");
        printf(" Assigned Slot  : Floor %d, Slot #%02d\n", floor, assignedSlot);
        printf(" Entry Time     : %s\n", timeBuf);
        printf(" Hourly Rate    : $%.2f / hr\n", get_hourly_rate(type));
        printf("=======================================================\n\n");
        return true;
    } else {
        /* No slot available, add to waiting queue */
        Vehicle v;
        strncpy(v.licensePlate, plate, PLATE_LEN - 1);
        v.licensePlate[PLATE_LEN - 1] = '\0';
        strncpy(v.ownerName, regVeh ? regVeh->ownerName : "Guest", NAME_LEN - 1);
        v.ownerName[NAME_LEN - 1] = '\0';
        strncpy(v.contactNumber, regVeh ? regVeh->contactNumber : "N/A", PHONE_LEN - 1);
        v.contactNumber[PHONE_LEN - 1] = '\0';
        v.type = type;
        v.registeredAt = entryTime;

        enqueue_waiting_vehicle(&(sys->waitingQueue), v);
        return false;
    }
}

bool process_vehicle_exit(ParkingSystem* sys, const char* plate, time_t customExit) {
    if (!sys || !plate) return false;

    int slotId = find_slot_by_plate(sys, plate);
    if (slotId == -1) {
        printf("[!] Vehicle '%s' is not found in any active parking slot.\n", plate);
        return false;
    }

    ParkingSlot* slot = &sys->slots[slotId - 1];
    time_t exitTime = (customExit != 0) ? customExit : time(NULL);

    if (exitTime < slot->entryTime) {
        exitTime = slot->entryTime; /* prevent negative duration */
    }

    double diffSeconds = difftime(exitTime, slot->entryTime);
    double durationHours = diffSeconds / 3600.0;
    if (durationHours < 0.05) {
        /* If tested immediately, default to 1.0 hour for demonstration */
        durationHours = 1.0;
    }

    /* Query AI demand prediction for current exit hour to get dynamic surge multiplier */
    struct tm* exitTm = localtime(&exitTime);
    int hour = exitTm ? exitTm->tm_hour : 12;
    double predictedDemand = 0.0;
    double surgeFactor = 1.0;
    predict_demand_for_hour(sys, hour, &predictedDemand, &surgeFactor);

    /* Look up vehicle type */
    Vehicle* regVeh = search_registered_vehicle(sys, plate);
    VehicleType vType = regVeh ? regVeh->type : slot->supportedType;

    double baseRate = get_hourly_rate(vType);
    double totalCharge = calculate_parking_charges(vType, durationHours, surgeFactor);

    /* Build Transaction record */
    Transaction tx;
    tx.ticketId = slot->ticketId;
    strncpy(tx.licensePlate, plate, PLATE_LEN - 1);
    tx.licensePlate[PLATE_LEN - 1] = '\0';
    tx.vehicleType = vType;
    tx.slotId = slot->slotId;
    tx.floor = slot->floor;
    tx.entryTime = slot->entryTime;
    tx.exitTime = exitTime;
    tx.durationHours = durationHours;
    tx.baseRate = baseRate;
    tx.surgeFactor = surgeFactor;
    tx.totalCharge = totalCharge;

    /* Add to history linked list */
    add_transaction_to_history(sys, tx);

    /* Print bill */
    print_bill_receipt(&tx);

    /* Free the slot in the array */
    free_slot(sys, slotId);
    printf("[+] Slot #%02d (Floor %d) is now VACANT.\n", slotId, slot->floor);

    /* AUTO-ALLOCATION: Check FIFO Waiting Queue to immediately occupy freed slot */
    if (sys->waitingQueue.count > 0) {
        Vehicle queuedVehicle;
        if (peek_waiting_vehicle(&(sys->waitingQueue), &queuedVehicle)) {
            /* Try allocating the freed slot or suitable slot for the waiting vehicle */
            int newSlot = allocate_slot_for_vehicle(sys, queuedVehicle.type,
                                                   queuedVehicle.licensePlate,
                                                   sys->nextTicketId++,
                                                   exitTime);
            if (newSlot != -1) {
                dequeue_waiting_vehicle(&(sys->waitingQueue), NULL);
                printf("\n>>> [AUTO-ALLOCATION]: Dequeued waiting vehicle '%s' and assigned to Slot #%02d! <<<\n\n",
                       queuedVehicle.licensePlate, newSlot);
            }
        }
    }

    return true;
}
