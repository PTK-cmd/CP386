#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_THREADS 100

// Structure to store thread information
typedef struct {
    int thread_id; // ID of the thread
    int y;         // Last digit of the thread ID
} ThreadInfo;

// Global Variables
ThreadInfo thread_array[MAX_THREADS];
int thread_count = 0;
int next_expected_y = -1; // -1 means no thread has entered critical section yet
pthread_mutex_t order_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t semaphore; // Semaphore to control critical section access

// Function to simulate critical section
void* threadRun(void* arg) {
    ThreadInfo* thread_info = (ThreadInfo*)arg;

    // Indicate thread start
    printf("[Thread %d] New Thread with ID t%03d is started.\n", thread_info->thread_id, thread_info->thread_id);

    // Wait for semaphore
    sem_wait(&semaphore);

    pthread_mutex_lock(&order_mutex);

    // Check if this thread's 'y' matches the expected pattern
    if (next_expected_y == -1 || (thread_info->y % 2 != next_expected_y % 2)) {
        // Enter critical section
        printf("[Thread %d] Thread t%03d is in its critical section.\n", thread_info->thread_id, thread_info->thread_id);
        next_expected_y = thread_info->y; // Update the next expected 'y'
    } else {
        printf("[Thread %d] Thread t%03d is waiting for its turn.\n", thread_info->thread_id, thread_info->thread_id);
    }

    pthread_mutex_unlock(&order_mutex);

    // Simulate work
    sleep(1);

    // Exit critical section
    printf("[Thread %d] Thread with ID t%03d is finished.\n", thread_info->thread_id, thread_info->thread_id);

    // Release semaphore
    sem_post(&semaphore);

    return NULL;
}

int main() {
    pthread_t threads[MAX_THREADS];

    // Initialize semaphore
    sem_init(&semaphore, 0, 1);

    // Read the input (Assume input format: Thread ID per line)
    printf("Enter the number of threads: ");
    scanf("%d", &thread_count);

    for (int i = 0; i < thread_count; i++) {
        int id;
        printf("Enter Thread ID for thread %d (txy format): ", i + 1);
        scanf("%d", &id);
        thread_array[i].thread_id = id;
        thread_array[i].y = id % 10; // Extract last digit
    }

    // Create threads
    for (int i = 0; i < thread_count; i++) {
        pthread_create(&threads[i], NULL, threadRun, &thread_array[i]);
        sleep(1); // Simulate staggered thread creation
    }

    // Wait for threads to complete
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    // Clean up
    sem_destroy(&semaphore);

    printf("All threads have completed execution.\n");
    return 0;
}
