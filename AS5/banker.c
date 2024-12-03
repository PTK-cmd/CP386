#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CUSTOMERS 5
#define MAX_RESOURCES 4

// Global variables
int available[MAX_RESOURCES];
int maximum[MAX_CUSTOMERS][MAX_RESOURCES];
int allocation[MAX_CUSTOMERS][MAX_RESOURCES];
int need[MAX_CUSTOMERS][MAX_RESOURCES];
int num_resources;
int num_customers;
int safe_sequence[MAX_CUSTOMERS];
int sequence_index = 0;

// Mutex for synchronization
pthread_mutex_t lock;

// Function to check if the system is in a safe state
bool is_safe_state() {
    int work[MAX_RESOURCES];
    bool finish[MAX_CUSTOMERS] = {false};
    memcpy(work, available, sizeof(available));
    sequence_index = 0;

    for (int i = 0; i < num_customers;) {
        bool found = false;
        for (int j = 0; j < num_customers; j++) {
            if (!finish[j]) {
                bool can_allocate = true;
                for (int k = 0; k < num_resources; k++) {
                    if (need[j][k] > work[k]) {
                        can_allocate = false;
                        break;
                    }
                }
                if (can_allocate) {
                    for (int k = 0; k < num_resources; k++) {
                        work[k] += allocation[j][k];
                    }
                    finish[j] = true;
                    safe_sequence[sequence_index++] = j;
                    found = true;
                }
            }
        }
        if (!found) break;
        i++;
    }

    for (int i = 0; i < num_customers; i++) {
        if (!finish[i]) return false;
    }
    return true;
}

// Request resources
void request_resources(int customer_num, int request[]) {
    pthread_mutex_lock(&lock);

    for (int i = 0; i < num_resources; i++) {
        if (request[i] > need[customer_num][i] || request[i] > available[i]) {
            printf("Request denied. Not safe.\n");
            pthread_mutex_unlock(&lock);
            return;
        }
    }

    for (int i = 0; i < num_resources; i++) {
        available[i] -= request[i];
        allocation[customer_num][i] += request[i];
        need[customer_num][i] -= request[i];
    }

    if (is_safe_state()) {
        printf("State is safe, and request is satisfied.\n");
    } else {
        printf("State is unsafe, rolling back.\n");
        for (int i = 0; i < num_resources; i++) {
            available[i] += request[i];
            allocation[customer_num][i] -= request[i];
            need[customer_num][i] += request[i];
        }
    }

    pthread_mutex_unlock(&lock);
}

// Release resources
void release_resources(int customer_num, int release[]) {
    pthread_mutex_lock(&lock);

    for (int i = 0; i < num_resources; i++) {
        allocation[customer_num][i] -= release[i];
        available[i] += release[i];
        need[customer_num][i] += release[i];
    }

    printf("The resources have been released successfully.\n");
    pthread_mutex_unlock(&lock);
}

// Print status
void print_status() {
    pthread_mutex_lock(&lock);

    printf("Available Resources:\n");
    for (int i = 0; i < num_resources; i++) {
        printf("%d ", available[i]);
    }
    printf("\n");

    printf("Maximum Resources:\n");
    for (int i = 0; i < num_customers; i++) {
        for (int j = 0; j < num_resources; j++) {
            printf("%d ", maximum[i][j]);
        }
        printf("\n");
    }

    printf("Allocated Resources:\n");
    for (int i = 0; i < num_customers; i++) {
        for (int j = 0; j < num_resources; j++) {
            printf("%d ", allocation[i][j]);
        }
        printf("\n");
    }

    printf("Need Resources:\n");
    for (int i = 0; i < num_customers; i++) {
        for (int j = 0; j < num_resources; j++) {
            printf("%d ", need[i][j]);
        }
        printf("\n");
    }

    pthread_mutex_unlock(&lock);
}

// Run the safe sequence
void* run_customer(void* arg) {
    int customer_num = *(int*)arg;

    printf("--> Customer/Thread %d\n", customer_num);
    printf("Allocated resources: ");
    for (int i = 0; i < num_resources; i++) {
        printf("%d ", allocation[customer_num][i]);
    }
    printf("\n");

    printf("Needed: ");
    for (int i = 0; i < num_resources; i++) {
        printf("%d ", need[customer_num][i]);
    }
    printf("\n");

    printf("Available: ");
    for (int i = 0; i < num_resources; i++) {
        printf("%d ", available[i]);
    }
    printf("\n");

    printf("Thread has started\n");

    for (int i = 0; i < num_resources; i++) {
        available[i] -= need[customer_num][i];
        allocation[customer_num][i] += need[customer_num][i];
        need[customer_num][i] = 0;
    }

    printf("Thread has finished\n");

    for (int i = 0; i < num_resources; i++) {
        available[i] += allocation[customer_num][i];
        allocation[customer_num][i] = 0;
    }

    printf("Thread is releasing resources\n");
    printf("New Available Resources: ");
    for (int i = 0; i < num_resources; i++) {
        printf("%d ", available[i]);
    }
    printf("\n");

    return NULL;
}

void run() {
    pthread_t threads[MAX_CUSTOMERS];
    int customer_nums[MAX_CUSTOMERS];

    if (!is_safe_state()) {
        printf("System is not in a safe state. Cannot run.\n");
        return;
    }

    printf("Safe Sequence is: ");
    for (int i = 0; i < sequence_index; i++) {
        printf("%d ", safe_sequence[i]);
    }
    printf("\n");

    for (int i = 0; i < sequence_index; i++) {
        customer_nums[i] = safe_sequence[i];
        pthread_create(&threads[i], NULL, run_customer, &customer_nums[i]);
    }

    for (int i = 0; i < sequence_index; i++) {
        pthread_join(threads[i], NULL);
    }
}

// Main function
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: ./banker <available_resources>\n");
        return -1;
    }

    num_resources = argc - 1;
    num_customers = MAX_CUSTOMERS;

    for (int i = 0; i < num_resources; i++) {
        available[i] = atoi(argv[i + 1]);
    }

    printf("Enter maximum matrix:\n");
    for (int i = 0; i < num_customers; i++) {
        for (int j = 0; j < num_resources; j++) {
            scanf("%d", &maximum[i][j]);
            need[i][j] = maximum[i][j];
        }
    }

    char command[100];
    while (1) {
        printf("Enter Command: ");
        scanf("%s", command);

        if (strcmp(command, "RQ") == 0) {
            int customer_num, request[MAX_RESOURCES];
            scanf("%d", &customer_num);
            for (int i = 0; i < num_resources; i++) {
                scanf("%d", &request[i]);
            }
            request_resources(customer_num, request);
        } else if (strcmp(command, "RL") == 0) {
            int customer_num, release[MAX_RESOURCES];
            scanf("%d", &customer_num);
            for (int i = 0; i < num_resources; i++) {
                scanf("%d", &release[i]);
            }
            release_resources(customer_num, release);
        } else if (strcmp(command, "Status") == 0) {
            print_status();
        } else if (strcmp(command, "Run") == 0) {
            run();
        } else if (strcmp(command, "Exit") == 0) {
            break;
        } else {
            printf("Invalid input. Use one of RQ, RL, Status, Run, Exit.\n");
        }
    }

    return 0;
}
