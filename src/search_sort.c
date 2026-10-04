#include "parking_system.h"

int linear_search_occupied_slot(const ParkingSystem* sys, const char* plate) {
    if (!sys || !plate) return -1;
    for (int i = 0; i < sys->totalSlots; i++) {
        if (sys->slots[i].status == SLOT_OCCUPIED &&
            plates_match(sys->slots[i].occupiedPlate, plate)) {
            return sys->slots[i].slotId;
        }
    }
    return -1;
}

Transaction** get_transactions_array(const ParkingSystem* sys, int* outCount) {
    if (!sys || !outCount) return NULL;

    *outCount = sys->totalTransactions;
    if (*outCount == 0) return NULL;

    Transaction** arr = (Transaction**)malloc(sizeof(Transaction*) * (*outCount));
    if (!arr) return NULL;

    TransactionNode* curr = sys->historyHead;
    int idx = 0;
    while (curr != NULL && idx < *outCount) {
        arr[idx++] = &(curr->data);
        curr = curr->next;
    }
    return arr;
}

/* Quick Sort: Partition by Total Charge */
static int partition_charge(Transaction** arr, int low, int high, bool ascending) {
    double pivot = arr[high]->totalCharge;
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        bool condition = ascending ? (arr[j]->totalCharge <= pivot)
                                   : (arr[j]->totalCharge >= pivot);
        if (condition) {
            i++;
            Transaction* temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    Transaction* temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return (i + 1);
}

void quick_sort_transactions_by_amount(Transaction** arr, int low, int high, bool ascending) {
    if (low < high) {
        int pi = partition_charge(arr, low, high, ascending);
        quick_sort_transactions_by_amount(arr, low, pi - 1, ascending);
        quick_sort_transactions_by_amount(arr, pi + 1, high, ascending);
    }
}

/* Merge Sort: Merge two subarrays by Entry Time */
static void merge_by_time(Transaction** arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    Transaction** L = (Transaction**)malloc(sizeof(Transaction*) * n1);
    Transaction** R = (Transaction**)malloc(sizeof(Transaction*) * n2);

    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i]->entryTime <= R[j]->entryTime) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

void merge_sort_transactions_by_time(Transaction** arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort_transactions_by_time(arr, left, mid);
        merge_sort_transactions_by_time(arr, mid + 1, right);
        merge_by_time(arr, left, mid, right);
    }
}

/* Binary Search: By Ticket ID on sorted array */
int binary_search_transaction_by_ticket(Transaction** sortedArr, int count, int ticketId) {
    int low = 0;
    int high = count - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (sortedArr[mid]->ticketId == ticketId) {
            return mid;
        }
        if (sortedArr[mid]->ticketId < ticketId) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}
