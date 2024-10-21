#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>

#define SHM_SIZE 1024 // Shared memory size
#define CMD_SIZE 256  // Command size
void write_output(const char *output) {
    FILE *file = fopen("output.txt", "a");
    if (file) {
        fprintf(file, "%s\n", output);
        fclose(file);
    } else {
        perror("Failed to open output file");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Failed to open output file\n");
        exit(1);
    }

    const char *input_file = argv[1];
    int shmid = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("Failed to create shared memory");
        exit(1);
    }

    char *shm_ptr = (char *)shmat(shmid, NULL, 0);
    if (shm_ptr == (char *)-1) {
        perror("Failed to attach shared memory");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Failed to fork child process");
        exit(1);
    }

    if (pid == 0) { // Child process reads file content
        FILE *file = fopen(input_file, "r");
        if (!file) {
            perror("Failed to open input file");
            exit(1);
        }

        size_t index = 0;
        char command[CMD_SIZE];
        while (fgets(command, sizeof(command), file)) {
            strcpy(&shm_ptr[index], command);
            index += strlen(command);
        }

        fclose(file);
        exit(0);
    } else { // Parent process
        wait(NULL); // Wait for the child process to finish

        char *commands = shm_ptr;
        char *command_line = strtok(commands, "\n");

        while (command_line != NULL) {
            int fd[2];
            pipe(fd);
            pid_t child_pid = fork();

            if (child_pid < 0) {
                perror("Failed to fork child process");
                exit(1);
            }

            if (child_pid == 0) { // Child process executes command
                dup2(fd[1], STDOUT_FILENO); // Redirect output to pipe
                close(fd[0]);
                close(fd[1]);

                char *args[CMD_SIZE];
                args[0] = strtok(command_line, " ");
                int i = 1;

                while ((args[i] = strtok(NULL, " ")) != NULL) {
                    i++;
                }
                
                execvp(args[0], args); // Execute the command
                perror("execfailed");
                exit(1);
            } else { // Parent process reads output and writes to file
                close(fd[1]); // Parent closes the write end of the pipe

                char output[CMD_SIZE];
                ssize_t bytes_read = read(fd[0], output, sizeof(output) - 1);
                if (bytes_read > 0) {
                    output[bytes_read] = '\0'; // null-terminate output
                    write_output(output);
                }

                close(fd[0]);
                wait(NULL); // Wait for the child process to finish
            }

            command_line = strtok(NULL, "\n"); // Get the next command line
        }

        shmdt(shm_ptr); // Detach shared memory
        shmctl(shmid, IPC_RMID, NULL); // Remove shared memory
    }
    
    return 0;
}