#include "parking_system.h"

void init_parking_system(ParkingSystem* sys) {
    if (!sys) return;
    memset(sys, 0, sizeof(ParkingSystem));
    sys->nextTicketId = 1001;
    sys->totalRevenueCollected = 0.0;
    sys->registryHead = NULL;
    sys->historyHead = NULL;

    init_parking_slots(sys);
    init_waiting_queue(&(sys->waitingQueue));
    train_ai_demand_model(sys);
}

void free_parking_system(ParkingSystem* sys) {
    if (!sys) return;
    free_vehicle_registry(sys);
    free_waiting_queue(&(sys->waitingQueue));
    free_transaction_history(sys);
}

static void print_main_menu(void) {
    printf("\n=========================================================================\n");
    printf("                  SMART PARKING MANAGEMENT SYSTEM                        \n");
    printf("=========================================================================\n");
    printf(" [1]  Vehicle Check-In (Entry & Slot Allocation)\n");
    printf(" [2]  Vehicle Check-Out (Exit, Billing & Queue Auto-Allocation)\n");
    printf(" [3]  Search Vehicle by License Plate (Linear Search)\n");
    printf(" [4]  View Real-Time Parking Slots Table & 2D Grid\n");
    printf(" [5]  View Waiting Queue (FIFO Queue)\n");
    printf(" [6]  Register New Vehicle (Linked List)\n");
    printf(" [7]  View Registered Vehicles Database\n");
    printf(" [8]  View Completed Billing History\n");
    printf(" [9]  Sort Billing History (QuickSort / MergeSort)\n");
    printf(" [10] AI Demand Prediction & Dynamic Surge Tariff\n");
    printf(" [11] Parking Operational Statistics & Revenue\n");
    printf(" [12] Run Full Automated End-to-End Demo Simulation\n");
    printf(" [13] Save System Data to Disk\n");
    printf(" [14] Load System Data from Disk\n");
    printf(" [0]  Exit System\n");
    printf("=========================================================================\n");
    printf("Enter choice (0-14): ");
}

int main(int argc, char* argv[]) {
    ParkingSystem system;
    init_parking_system(&system);

    /* Try loading saved state on startup if available */
    load_system_state(&system, "data/parking_state.txt");

    /* Quick headless demo mode if launched with --demo */
    if (argc > 1 && (strcmp(argv[1], "--demo") == 0 || strcmp(argv[1], "-d") == 0)) {
        run_automated_demo(&system);
        free_parking_system(&system);
        return 0;
    }

    int choice = -1;
    char buffer[128];

    while (1) {
        print_main_menu();
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        if (sscanf(buffer, "%d", &choice) != 1) {
            printf("[!] Invalid input. Please enter a valid number.\n");
            continue;
        }

        if (choice == 0) {
            printf("Saving state and exiting...\n");
            save_system_state(&system, "data/parking_state.txt");
            break;
        }

        switch (choice) {
            case 1: {
                char plate[PLATE_LEN], owner[NAME_LEN], phone[PHONE_LEN];
                int vTypeInt = 2;

                printf("\n--- Vehicle Check-In ---\n");
                printf("Enter License Plate (e.g. KA-01-AB-1234): ");
                if (!fgets(plate, sizeof(plate), stdin)) break;
                plate[strcspn(plate, "\r\n")] = '\0';

                printf("Enter Owner Name: ");
                if (!fgets(owner, sizeof(owner), stdin)) break;
                owner[strcspn(owner, "\r\n")] = '\0';

                printf("Enter Contact Number: ");
                if (!fgets(phone, sizeof(phone), stdin)) break;
                phone[strcspn(phone, "\r\n")] = '\0';

                printf("Select Vehicle Type:\n");
                printf("  [1] Two-Wheeler (Motorcycle/Scooter)\n");
                printf("  [2] Four-Wheeler (Car/Sedan/SUV)\n");
                printf("  [3] Heavy Vehicle (Truck/Bus)\n");
                printf("Choice (1-3): ");
                if (fgets(buffer, sizeof(buffer), stdin)) {
                    sscanf(buffer, "%d", &vTypeInt);
                }
                if (vTypeInt < 1 || vTypeInt > 3) vTypeInt = 2;

                process_vehicle_entry(&system, plate, owner, phone, (VehicleType)vTypeInt, 0);
                break;
            }

            case 2: {
                char plate[PLATE_LEN];
                printf("\n--- Vehicle Check-Out & Billing ---\n");
                printf("Enter License Plate: ");
                if (!fgets(plate, sizeof(plate), stdin)) break;
                plate[strcspn(plate, "\r\n")] = '\0';

                process_vehicle_exit(&system, plate, 0);
                break;
            }

            case 3: {
                char plate[PLATE_LEN];
                printf("\n--- Search Slot by License Plate ---\n");
                printf("Enter License Plate: ");
                if (!fgets(plate, sizeof(plate), stdin)) break;
                plate[strcspn(plate, "\r\n")] = '\0';

                int slot = linear_search_occupied_slot(&system, plate);
                if (slot != -1) {
                    printf("[+] FOUND: Vehicle '%s' is parked in Slot #%02d (Floor %d).\n",
                           plate, slot, system.slots[slot - 1].floor);
                } else {
                    printf("[-] Vehicle '%s' is not found in any active slot.\n", plate);
                }
                break;
            }

            case 4:
                display_slots_table(&system);
                display_parking_grid_ascii(&system);
                break;

            case 5:
                display_waiting_queue(&(system.waitingQueue));
                break;

            case 6: {
                char plate[PLATE_LEN], owner[NAME_LEN], phone[PHONE_LEN];
                int vTypeInt = 2;

                printf("\n--- Register Vehicle ---\n");
                printf("Enter License Plate: ");
                if (!fgets(plate, sizeof(plate), stdin)) break;
                plate[strcspn(plate, "\r\n")] = '\0';

                printf("Enter Owner Name: ");
                if (!fgets(owner, sizeof(owner), stdin)) break;
                owner[strcspn(owner, "\r\n")] = '\0';

                printf("Enter Contact Number: ");
                if (!fgets(phone, sizeof(phone), stdin)) break;
                phone[strcspn(phone, "\r\n")] = '\0';

                printf("Vehicle Type (1: 2-Wheeler, 2: 4-Wheeler, 3: Heavy): ");
                if (fgets(buffer, sizeof(buffer), stdin)) {
                    sscanf(buffer, "%d", &vTypeInt);
                }
                if (vTypeInt < 1 || vTypeInt > 3) vTypeInt = 2;

                register_vehicle(&system, plate, owner, phone, (VehicleType)vTypeInt);
                break;
            }

            case 7:
                display_registered_vehicles(&system);
                break;

            case 8:
                display_transaction_history(&system);
                break;

            case 9: {
                int count = 0;
                Transaction** arr = get_transactions_array(&system, &count);
                if (!arr || count == 0) {
                    printf("[!] No completed transactions available to sort.\n");
                    break;
                }

                printf("\nSelect Sorting Metric:\n");
                printf("  [1] QuickSort by Billing Amount (Highest to Lowest)\n");
                printf("  [2] QuickSort by Billing Amount (Lowest to Highest)\n");
                printf("  [3] MergeSort by Entry Timestamp\n");
                printf("Choice (1-3): ");
                int sortChoice = 1;
                if (fgets(buffer, sizeof(buffer), stdin)) {
                    sscanf(buffer, "%d", &sortChoice);
                }

                if (sortChoice == 1) {
                    quick_sort_transactions_by_amount(arr, 0, count - 1, false);
                    printf("\n--- Transactions Sorted by Amount (Descending) ---\n");
                } else if (sortChoice == 2) {
                    quick_sort_transactions_by_amount(arr, 0, count - 1, true);
                    printf("\n--- Transactions Sorted by Amount (Ascending) ---\n");
                } else {
                    merge_sort_transactions_by_time(arr, 0, count - 1);
                    printf("\n--- Transactions Sorted by Entry Time ---\n");
                }

                for (int i = 0; i < count; i++) {
                    char tBuf[TIME_STR_LEN];
                    format_timestamp(arr[i]->entryTime, tBuf, sizeof(tBuf));
                    printf("#%04d | %-14s | Entry: %-19s | Total: $%.2f\n",
                           arr[i]->ticketId, arr[i]->licensePlate, tBuf, arr[i]->totalCharge);
                }
                free(arr);
                break;
            }

            case 10:
                display_ai_demand_dashboard(&system);
                break;

            case 11:
                display_parking_statistics(&system);
                break;

            case 12:
                run_automated_demo(&system);
                break;

            case 13:
                save_system_state(&system, "data/parking_state.txt");
                break;

            case 14:
                load_system_state(&system, "data/parking_state.txt");
                break;

            default:
                printf("[!] Invalid choice. Please select from 0 to 14.\n");
                break;
        }
    }

    free_parking_system(&system);
    printf("Goodbye!\n");
    return 0;
}
