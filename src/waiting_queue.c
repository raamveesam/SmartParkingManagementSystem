#include "parking_system.h"

void init_waiting_queue(WaitingQueue* q) {
    if (!q) return;
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
}

bool enqueue_waiting_vehicle(WaitingQueue* q, Vehicle vehicle) {
    if (!q) return false;

    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    if (!newNode) {
        fprintf(stderr, "Error: Failed to allocate memory for waiting queue node.\n");
        return false;
    }

    newNode->vehicle = vehicle;
    newNode->requestTime = time(NULL);
    newNode->next = NULL;

    if (q->rear == NULL) {
        /* Queue is currently empty */
        q->front = newNode;
        q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }

    q->count++;
    printf("[i] Parking full! Vehicle '%s' added to FIFO Waiting Queue (Position #%d).\n",
           vehicle.licensePlate, q->count);
    return true;
}

bool dequeue_waiting_vehicle(WaitingQueue* q, Vehicle* outVehicle) {
    if (!q || q->front == NULL) {
        return false;
    }

    QueueNode* temp = q->front;
    if (outVehicle) {
        *outVehicle = temp->vehicle;
    }

    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }

    free(temp);
    q->count--;
    return true;
}

bool peek_waiting_vehicle(const WaitingQueue* q, Vehicle* outVehicle) {
    if (!q || q->front == NULL) {
        return false;
    }
    if (outVehicle) {
        *outVehicle = q->front->vehicle;
    }
    return true;
}

void display_waiting_queue(const WaitingQueue* q) {
    if (!q) return;

    printf("\n=========================================================================================\n");
    printf("                                FIFO WAITING QUEUE STATUS                                \n");
    printf("=========================================================================================\n");
    printf("%-5s | %-15s | %-20s | %-20s | %-20s\n",
           "Pos", "License Plate", "Vehicle Type", "Owner Name", "Queued Time");
    printf("-----------------------------------------------------------------------------------------\n");

    if (q->front == NULL) {
        printf("  [Waiting queue is empty. No vehicles currently queued.]\n");
    } else {
        QueueNode* curr = q->front;
        int pos = 1;
        char timeBuf[TIME_STR_LEN];
        while (curr != NULL) {
            format_timestamp(curr->requestTime, timeBuf, sizeof(timeBuf));
            printf("#%-4d | %-15s | %-20s | %-20s | %-20s\n",
                   pos,
                   curr->vehicle.licensePlate,
                   vehicle_type_to_string(curr->vehicle.type),
                   curr->vehicle.ownerName,
                   timeBuf);
            curr = curr->next;
            pos++;
        }
    }
    printf("-----------------------------------------------------------------------------------------\n");
    printf("Total waiting in queue: %d\n\n", q->count);
}

void free_waiting_queue(WaitingQueue* q) {
    if (!q) return;
    QueueNode* curr = q->front;
    while (curr != NULL) {
        QueueNode* temp = curr;
        curr = curr->next;
        free(temp);
    }
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
}
