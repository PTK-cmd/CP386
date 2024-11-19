#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_RESOURCES 5
#define NUM_THREADS 5

int available_resources = MAX_RESOURCES;
pthread_mutex_t resource_mutex;

// Decrease available resources
int decrease_count(int thread_number, int count) {
    pthread_mutex_lock(&resource_mutex); // Lock critical section
    if (available_resources < count) {
        pthread_mutex_unlock(&resource_mutex); // Unlock if resources are insufficient
        printf("Thread %d could not acquire enough resources.\n", thread_number);
        return -1;
    } else {
        available_resources -= count;
        printf("Thread %d has acquired %d resources. %d resources are available.\n", 
                thread_number, count, available_resources);
        pthread_mutex_unlock(&resource_mutex); // Unlock after successful allocation
        return 0;
    }
}

// Increase available resources
void increase_count(int thread_number, int count) {
    pthread_mutex_lock(&resource_mutex); // Lock critical section
    available_resources += count;
    printf("Thread %d has released %d resources. %d resources are now available.\n", 
            thread_number, count, available_resources);
    pthread_mutex_unlock(&resource_mutex); // Unlock after releasing resources
}

// Thread function
void* thread_function(void* arg) {
    int thread_number = *(int*)arg;
    int count = rand() % MAX_RESOURCES + 1; // Randomly request between 1 and MAX_RESOURCES

    if (decrease_count(thread_number, count) == 0) {
        sleep(1); // Simulate thread work
        increase_count(thread_number, count);
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_numbers[NUM_THREADS];
    pthread_mutex_init(&resource_mutex, NULL); // Initialize mutex

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_numbers[i] = i + 1;
        pthread_create(&threads[i], NULL, thread_function, &thread_numbers[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All threads have finished execution. Available resources: %d\n", available_resources);

    pthread_mutex_destroy(&resource_mutex); // Destroy mutex
    return 0;
}
