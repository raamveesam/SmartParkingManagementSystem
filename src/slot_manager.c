#include "parking_system.h"

void init_parking_slots(ParkingSystem* sys) {
    if (!sys) return;

    sys->totalSlots = MAX_SLOTS;

    for (int i = 0; i < MAX_SLOTS; i++) {
        sys->slots[i].slotId = i + 1;
        sys->slots[i].floor = (i / SLOTS_PER_FLOOR) + 1;
        sys->slots[i].status = SLOT_VACANT;
        sys->slots[i].occupiedPlate[0] = '\0';
        sys->slots[i].ticketId = 0;
        sys->slots[i].entryTime = 0;

        /* Assign designated slot types per floor */
        if (i < 4) {
            /* Floor 1: First 4 slots for 2-wheelers */
            sys->slots[i].supportedType = TYPE_TWO_WHEELER;
        } else if (i < 25) {
            /* Floor 1 (rest) and Floor 2 and first half of Floor 3 for 4-wheelers */
            sys->slots[i].supportedType = TYPE_FOUR_WHEELER;
        } else {
            /* Floor 3: Slots 26-30 for Heavy Vehicles */
            sys->slots[i].supportedType = TYPE_HEAVY_VEHICLE;
        }
    }
}

int allocate_slot_for_vehicle(ParkingSystem* sys, VehicleType type, const char* plate, int ticketId, time_t entryTime) {
    if (!sys || !plate) return -1;

    int assignedIndex = -1;

    /* Primary search: find exact designated vacant slot */
    for (int i = 0; i < sys->totalSlots; i++) {
        if (sys->slots[i].status == SLOT_VACANT && sys->slots[i].supportedType == type) {
            assignedIndex = i;
            break;
        }
    }

    /* Fallback search: 2-wheelers can occupy a 4-wheeler slot if no 2-wheeler slots left */
    if (assignedIndex == -1 && type == TYPE_TWO_WHEELER) {
        for (int i = 0; i < sys->totalSlots; i++) {
            if (sys->slots[i].status == SLOT_VACANT && sys->slots[i].supportedType == TYPE_FOUR_WHEELER) {
                assignedIndex = i;
                break;
            }
        }
    }

    if (assignedIndex != -1) {
        sys->slots[assignedIndex].status = SLOT_OCCUPIED;
        strncpy(sys->slots[assignedIndex].occupiedPlate, plate, PLATE_LEN - 1);
        sys->slots[assignedIndex].occupiedPlate[PLATE_LEN - 1] = '\0';
        sys->slots[assignedIndex].ticketId = ticketId;
        sys->slots[assignedIndex].entryTime = entryTime;
        return sys->slots[assignedIndex].slotId;
    }

    /* No slot available for this vehicle type */
    return -1;
}

bool free_slot(ParkingSystem* sys, int slotId) {
    if (!sys || slotId < 1 || slotId > sys->totalSlots) return false;

    int idx = slotId - 1;
    if (sys->slots[idx].status != SLOT_OCCUPIED) {
        return false;
    }

    sys->slots[idx].status = SLOT_VACANT;
    sys->slots[idx].occupiedPlate[0] = '\0';
    sys->slots[idx].ticketId = 0;
    sys->slots[idx].entryTime = 0;
    return true;
}

int find_slot_by_plate(const ParkingSystem* sys, const char* plate) {
    if (!sys || !plate) return -1;

    for (int i = 0; i < sys->totalSlots; i++) {
        if (sys->slots[i].status == SLOT_OCCUPIED &&
            plates_match(sys->slots[i].occupiedPlate, plate)) {
            return sys->slots[i].slotId;
        }
    }
    return -1;
}

int count_available_slots_by_type(const ParkingSystem* sys, VehicleType type) {
    if (!sys) return 0;
    int count = 0;
    for (int i = 0; i < sys->totalSlots; i++) {
        if (sys->slots[i].status == SLOT_VACANT) {
            if (sys->slots[i].supportedType == type ||
               (type == TYPE_TWO_WHEELER && sys->slots[i].supportedType == TYPE_FOUR_WHEELER)) {
                count++;
            }
        }
    }
    return count;
}

void display_slots_table(const ParkingSystem* sys) {
    if (!sys) return;

    printf("\n===============================================================================================\n");
    printf("                                   CURRENT PARKING SLOTS STATUS (ARRAY)                        \n");
    printf("===============================================================================================\n");
    printf("%-7s | %-6s | %-16s | %-10s | %-14s | %-8s | %-20s\n",
           "Slot #", "Floor", "Slot Category", "Status", "Occupied Plate", "Ticket", "Entry Time");
    printf("-----------------------------------------------------------------------------------------------\n");

    char timeBuf[TIME_STR_LEN];
    for (int i = 0; i < sys->totalSlots; i++) {
        const ParkingSlot* s = &sys->slots[i];
        format_timestamp(s->entryTime, timeBuf, sizeof(timeBuf));
        const char* typeStr = (s->supportedType == TYPE_TWO_WHEELER) ? "2-Wheeler" :
                              (s->supportedType == TYPE_FOUR_WHEELER) ? "4-Wheeler" : "Heavy";
        const char* plateStr = (s->status == SLOT_OCCUPIED) ? s->occupiedPlate : "---";
        char ticketStr[16];
        if (s->status == SLOT_OCCUPIED) {
            snprintf(ticketStr, sizeof(ticketStr), "#%04d", s->ticketId);
        } else {
            snprintf(ticketStr, sizeof(ticketStr), "---");
        }

        printf("Slot %02d | Fl %d   | %-16s | %-10s | %-14s | %-8s | %-20s\n",
               s->slotId, s->floor, typeStr, slot_status_to_string(s->status),
               plateStr, ticketStr, (s->status == SLOT_OCCUPIED ? timeBuf : "---"));
    }
    printf("-----------------------------------------------------------------------------------------------\n\n");
}

void display_parking_grid_ascii(const ParkingSystem* sys) {
    if (!sys) return;

    printf("\n+===========================================================================+\n");
    printf("|                    PARKING LOT REAL-TIME 2D GRID MAP                     |\n");
    printf("+===========================================================================+\n");

    for (int f = 1; f <= TOTAL_FLOORS; f++) {
        printf("| FLOOR %d:                                                                   |\n", f);
        printf("|   ");
        for (int i = 0; i < sys->totalSlots; i++) {
            if (sys->slots[i].floor == f) {
                if (sys->slots[i].status == SLOT_VACANT) {
                    printf("[%02d: V ] ", sys->slots[i].slotId);
                } else if (sys->slots[i].status == SLOT_OCCUPIED) {
                    printf("[%02d: O ] ", sys->slots[i].slotId);
                } else {
                    printf("[%02d: M ] ", sys->slots[i].slotId);
                }
            }
        }
        printf("|\n");
    }
    printf("+---------------------------------------------------------------------------+\n");
    printf("| Legend: [ V ] = Vacant (Available)  |  [ O ] = Occupied  |  [ M ] = Maint |\n");
    printf("+===========================================================================+\n\n");
}
