#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

// Function to generate the Collatz sequence
void generate_collatz_sequence(int n, int* sequence, int* len) {
    int i = 0;
    while (n != 1) {
        sequence[i++] = n;
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            n = 3 * n + 1;
        }
    }
    sequence[i++] = 1;
    *len = i;
}

int main() {
    // Open the file containing the start numbers
    FILE* file = fopen("start_numbers.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    int start_number;
    while (fscanf(file, "%d", &start_number) == 1) {
        // Create a shared memory object
        int shm_fd = shm_open("collatz_shm", O_RDWR | O_CREAT, 0666);
        if (shm_fd == -1) {
            perror("Error creating shared memory object");
            exit(1);
        }

        // Set the size of the shared memory object
        ftruncate(shm_fd, sizeof(int) * 100); // Assuming a maximum sequence length of 100

        // Map the shared memory object to the process's address space
        int* shm_ptr = mmap(NULL, sizeof(int) * 100, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shm_ptr == MAP_FAILED) {
            perror("Error mapping shared memory object");
            exit(1);
        }

        // Generate the Collatz sequence
        int sequence_len;
        generate_collatz_sequence(start_number, shm_ptr, &sequence_len);

        // Print the parent message before forking
        printf("Parent Process: The positive integer read from file is %d\n", start_number);

        // Create a child process
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error creating child process");
            exit(1);
        }

        if (pid == 0) { // Child process
            // Output the contents of the shared memory object
            printf("Child Process: The generated collatz sequence is ");
            for (int i = 0; i < sequence_len; i++) {
                printf("%d ", shm_ptr[i]);
            }
            printf("\n");

            // Remove the shared memory object
            shm_unlink("collatz_shm");
            exit(0);
        } else { // Parent process
            // Wait for the child process to terminate
            wait(NULL);
        }
    }

    fclose(file);
    return 0;
}