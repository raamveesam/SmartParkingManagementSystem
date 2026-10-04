#include "parking_system.h"

const char* vehicle_type_to_string(VehicleType type) {
    switch (type) {
        case TYPE_TWO_WHEELER:
            return "2-Wheeler (Motorcycle/Scooter)";
        case TYPE_FOUR_WHEELER:
            return "4-Wheeler (Car/Sedan/SUV)";
        case TYPE_HEAVY_VEHICLE:
            return "Heavy (Truck/Bus/Van)";
        default:
            return "Unknown";
    }
}

const char* slot_status_to_string(SlotStatus status) {
    switch (status) {
        case SLOT_VACANT:
            return "VACANT";
        case SLOT_OCCUPIED:
            return "OCCUPIED";
        case SLOT_MAINTENANCE:
            return "MAINTENANCE";
        default:
            return "UNKNOWN";
    }
}

void format_timestamp(time_t rawTime, char* buffer, size_t maxLen) {
    if (rawTime == 0) {
        snprintf(buffer, maxLen, "N/A");
        return;
    }
    struct tm* timeInfo = localtime(&rawTime);
    if (timeInfo) {
        strftime(buffer, maxLen, "%Y-%m-%d %H:%M:%S", timeInfo);
    } else {
        snprintf(buffer, maxLen, "Invalid Time");
    }
}

bool plates_match(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        char ca = *a;
        char cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca += ('a' - 'A');
        if (cb >= 'A' && cb <= 'Z') cb += ('a' - 'A');
        if (ca != cb) return false;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

