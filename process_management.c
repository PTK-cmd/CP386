#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#define SHM_NAME "/shm_commands"
#define SHM_SIZE 4096

void write_output(const char *filename, const char *output) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%s\n", output);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *input_file = argv[1];
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    ftruncate(shm_fd, SHM_SIZE);
    char *shm_ptr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        FILE *file = fopen(input_file, "r");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        char command[256];
        while (fgets(command, sizeof(command), file) != NULL) {
            strcat(shm_ptr, command);
        }
        fclose(file);
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
        char *commands = strdup(shm_ptr);
        char *command = strtok(commands, "\n");
        while (command != NULL) {
            int pipe_fd[2];
            if (pipe(pipe_fd) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
            pid_t cmd_pid = fork();
            if (cmd_pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (cmd_pid == 0) {
                close(pipe_fd[0]);
                dup2(pipe_fd[1], STDOUT_FILENO);
                dup2(pipe_fd[1], STDERR_FILENO);
                close(pipe_fd[1]);
                char *args[] = {"/bin/sh", "-c", command, NULL};
                execvp(args[0], args);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else {
                close(pipe_fd[1]);
                wait(NULL);
                char buffer[1024];
                read(pipe_fd[0], buffer, sizeof(buffer));
                close(pipe_fd[0]);
                write_output("output.txt", buffer);
            }
            command = strtok(NULL, "\n");
        }
        free(commands);
        shm_unlink(SHM_NAME);
    }
    return 0;
}

