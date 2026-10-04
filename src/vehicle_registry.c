#include "parking_system.h"

bool register_vehicle(ParkingSystem* sys, const char* plate, const char* owner, const char* phone, VehicleType type) {
    if (!sys || !plate || strlen(plate) == 0) return false;

    /* Check if already registered */
    if (search_registered_vehicle(sys, plate) != NULL) {
        printf("[!] Vehicle with license plate '%s' is already registered.\n", plate);
        return false;
    }

    VehicleNode* newNode = (VehicleNode*)malloc(sizeof(VehicleNode));
    if (!newNode) {
        fprintf(stderr, "Error: Memory allocation failed for vehicle registration.\n");
        return false;
    }

    strncpy(newNode->vehicle.licensePlate, plate, PLATE_LEN - 1);
    newNode->vehicle.licensePlate[PLATE_LEN - 1] = '\0';

    strncpy(newNode->vehicle.ownerName, owner ? owner : "Guest", NAME_LEN - 1);
    newNode->vehicle.ownerName[NAME_LEN - 1] = '\0';

    strncpy(newNode->vehicle.contactNumber, phone ? phone : "N/A", PHONE_LEN - 1);
    newNode->vehicle.contactNumber[PHONE_LEN - 1] = '\0';

    newNode->vehicle.type = type;
    newNode->vehicle.registeredAt = time(NULL);
    newNode->next = NULL;

    /* Insert at the beginning of the Linked List (O(1)) */
    newNode->next = sys->registryHead;
    sys->registryHead = newNode;

    printf("[+] Successfully registered vehicle '%s' (%s, Owner: %s)\n",
           newNode->vehicle.licensePlate,
           vehicle_type_to_string(newNode->vehicle.type),
           newNode->vehicle.ownerName);

    return true;
}

Vehicle* search_registered_vehicle(ParkingSystem* sys, const char* plate) {
    if (!sys || !plate) return NULL;

    VehicleNode* curr = sys->registryHead;
    while (curr != NULL) {
        if (plates_match(curr->vehicle.licensePlate, plate)) {
            return &(curr->vehicle);
        }
        curr = curr->next;
    }
    return NULL;
}

void display_registered_vehicles(const ParkingSystem* sys) {
    if (!sys) return;

    printf("\n=========================================================================================\n");
    printf("                                REGISTERED VEHICLES (LINKED LIST)                       \n");
    printf("=========================================================================================\n");
    printf("%-15s | %-22s | %-20s | %-15s\n", "License Plate", "Owner Name", "Vehicle Type", "Contact");
    printf("-----------------------------------------------------------------------------------------\n");

    VehicleNode* curr = sys->registryHead;
    int count = 0;
    while (curr != NULL) {
        printf("%-15s | %-22s | %-20s | %-15s\n",
               curr->vehicle.licensePlate,
               curr->vehicle.ownerName,
               vehicle_type_to_string(curr->vehicle.type),
               curr->vehicle.contactNumber);
        curr = curr->next;
        count++;
    }

    if (count == 0) {
        printf("  [No registered vehicles found]\n");
    }
    printf("-----------------------------------------------------------------------------------------\n");
    printf("Total registered vehicles: %d\n\n", count);
}

void free_vehicle_registry(ParkingSystem* sys) {
    if (!sys) return;
    VehicleNode* curr = sys->registryHead;
    while (curr != NULL) {
        VehicleNode* temp = curr;
        curr = curr->next;
        free(temp);
    }
    sys->registryHead = NULL;
}
