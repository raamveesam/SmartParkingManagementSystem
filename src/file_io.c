#include "parking_system.h"

bool save_system_state(const ParkingSystem* sys, const char* filename) {
    if (!sys || !filename) return false;

    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("[!] Unable to open file for saving state");
        return false;
    }

    fprintf(fp, "# SMART PARKING MANAGEMENT SYSTEM PERSISTENCE FILE\n");
    fprintf(fp, "# METADATA\n");
    fprintf(fp, "NEXT_TICKET_ID=%d\n", sys->nextTicketId);
    fprintf(fp, "TOTAL_REVENUE=%.2f\n\n", sys->totalRevenueCollected);

    /* 1. Save Registered Vehicles */
    fprintf(fp, "[VEHICLE_REGISTRY]\n");
    VehicleNode* vNode = sys->registryHead;
    while (vNode != NULL) {
        fprintf(fp, "%s,%s,%s,%d,%ld\n",
                vNode->vehicle.licensePlate,
                vNode->vehicle.ownerName,
                vNode->vehicle.contactNumber,
                (int)vNode->vehicle.type,
                (long)vNode->vehicle.registeredAt);
        vNode = vNode->next;
    }
    fprintf(fp, "[END_VEHICLE_REGISTRY]\n\n");

    /* 2. Save Occupied Slots */
    fprintf(fp, "[ACTIVE_SLOTS]\n");
    for (int i = 0; i < sys->totalSlots; i++) {
        const ParkingSlot* s = &sys->slots[i];
        if (s->status == SLOT_OCCUPIED) {
            fprintf(fp, "%d,%d,%d,%s,%d,%ld\n",
                    s->slotId,
                    s->floor,
                    (int)s->supportedType,
                    s->occupiedPlate,
                    s->ticketId,
                    (long)s->entryTime);
        }
    }
    fprintf(fp, "[END_ACTIVE_SLOTS]\n\n");

    /* 3. Save Completed Transactions */
    fprintf(fp, "[TRANSACTIONS]\n");
    TransactionNode* tNode = sys->historyHead;
    while (tNode != NULL) {
        const Transaction* t = &(tNode->data);
        fprintf(fp, "%d,%s,%d,%d,%d,%ld,%ld,%.2f,%.2f,%.2f,%.2f\n",
                t->ticketId,
                t->licensePlate,
                (int)t->vehicleType,
                t->slotId,
                t->floor,
                (long)t->entryTime,
                (long)t->exitTime,
                t->durationHours,
                t->baseRate,
                t->surgeFactor,
                t->totalCharge);
        tNode = tNode->next;
    }
    fprintf(fp, "[END_TRANSACTIONS]\n");

    fclose(fp);
    printf("[+] System state successfully saved to '%s'.\n", filename);
    return true;
}

bool load_system_state(ParkingSystem* sys, const char* filename) {
    if (!sys || !filename) return false;

    FILE* fp = fopen(filename, "r");
    if (!fp) {
        /* If file doesn't exist, this is fine on first run */
        return false;
    }

    char line[512];
    char section[64] = "";

    while (fgets(line, sizeof(line), fp)) {
        /* Strip newline characters */
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '#' || line[0] == '\0') continue;

        if (strncmp(line, "NEXT_TICKET_ID=", 15) == 0) {
            sys->nextTicketId = atoi(line + 15);
            continue;
        }
        if (strncmp(line, "TOTAL_REVENUE=", 14) == 0) {
            sys->totalRevenueCollected = atof(line + 14);
            continue;
        }

        if (line[0] == '[') {
            strncpy(section, line, sizeof(section) - 1);
            continue;
        }

        if (strcmp(section, "[VEHICLE_REGISTRY]") == 0) {
            char plate[PLATE_LEN], owner[NAME_LEN], phone[PHONE_LEN];
            int typeInt;
            long regTime;
            if (sscanf(line, "%19[^,],%49[^,],%19[^,],%d,%ld",
                       plate, owner, phone, &typeInt, &regTime) == 5) {
                register_vehicle(sys, plate, owner, phone, (VehicleType)typeInt);
            }
        } else if (strcmp(section, "[ACTIVE_SLOTS]") == 0) {
            int slotId, floor, typeInt, ticketId;
            char plate[PLATE_LEN];
            long entryTime;
            if (sscanf(line, "%d,%d,%d,%19[^,],%d,%ld",
                       &slotId, &floor, &typeInt, plate, &ticketId, &entryTime) == 6) {
                if (slotId >= 1 && slotId <= sys->totalSlots) {
                    ParkingSlot* s = &sys->slots[slotId - 1];
                    s->status = SLOT_OCCUPIED;
                    strncpy(s->occupiedPlate, plate, PLATE_LEN - 1);
                    s->ticketId = ticketId;
                    s->entryTime = (time_t)entryTime;
                }
            }
        } else if (strcmp(section, "[TRANSACTIONS]") == 0) {
            Transaction tx;
            int typeInt;
            long entryT, exitT;
            if (sscanf(line, "%d,%19[^,],%d,%d,%d,%ld,%ld,%lf,%lf,%lf,%lf",
                       &tx.ticketId, tx.licensePlate, &typeInt, &tx.slotId, &tx.floor,
                       &entryT, &exitT, &tx.durationHours, &tx.baseRate,
                       &tx.surgeFactor, &tx.totalCharge) == 11) {
                tx.vehicleType = (VehicleType)typeInt;
                tx.entryTime = (time_t)entryT;
                tx.exitTime = (time_t)exitT;
                add_transaction_to_history(sys, tx);
            }
        }
    }

    fclose(fp);
    printf("[+] System state successfully restored from '%s'.\n", filename);
    return true;
}
