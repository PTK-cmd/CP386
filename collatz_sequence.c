#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SHM_NAME "/collatz_shm"
#define MAX_SEQUENCE_LENGTH 100

void collatz_sequence(int n, int *sequence, int *length) {
    *length = 0;
    while (n != 1) {
        sequence[(*length)++] = n;
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            n = 3 * n + 1;
        }
    }
    sequence[(*length)++] = 1; // Add 1 at the end
}

int main() {
    FILE *file = fopen("start_numbers.txt", "r");
    if (file == NULL) {
        perror("Unable to open file");
        return EXIT_FAILURE;
    }

    int start_number;
    pid_t pid;

    // Read each starting number from the file
    while (fscanf(file, "%d", &start_number) != EOF) {
        // Create shared memory object
        int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        if (shm_fd < 0) {
            perror("Failed to create shared memory");
            exit(1);
        }

        //  Set the size of the shared memory
        ftruncate(shm_fd, sizeof(int) * MAX_SEQUENCE_LENGTH);

        // Map the shared memory
        int *sequence = (int *)mmap(0, sizeof(int) * MAX_SEQUENCE_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (sequence == MAP_FAILED) {
            perror("Failed to map shared memory");
            exit(1);
        }

        // Create Child process
        pid = fork();

        if (pid < 0) {
            perror("Failed to create child process");
            exit(1);
        } else if (pid == 0) {
            // Child process
            int length;
            collatz_sequence(start_number, sequence, &length);
            
            // Output the sequence from shared memory
            printf("Child process output Collatz Sequence (start: %d): ", start_number);
            for (int i = 0; i < length; i++) {
                printf("%d ", sequence[i]);
            }
            printf("\n");
            
            // Clean up shared memory
            munmap(sequence, sizeof(int) * MAX_SEQUENCE_LENGTH);
            shm_unlink(SHM_NAME); // Remove shared memory object
            exit(0);
        } else {
            // Parent process
            wait(NULL); //  Wait for the child process to finish
        }

        // Clean up shared memory
        munmap(sequence, sizeof(int) * MAX_SEQUENCE_LENGTH);
    }

    fclose(file);
    return 0;
}
