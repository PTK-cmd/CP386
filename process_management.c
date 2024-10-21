#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/mman.h>

#define BUFSIZE 4096
#define SHMSIZE 1024
#define MAX_LINE_LENGTH 256

// utility to print the output
void writeOutput(char *command, char *output)
{
    FILE *fp;
    fp = fopen("output.txt", "a");

    fprintf(fp, "The output of: %s : is\n", command);
    fprintf(fp, ">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<\n", output);

    fclose(fp);
}

int main()
{
    int shmid;
    char *shm;
    // Open shared memory
    if ((shmid = shm_open("/my_shm", O_CREAT | O_RDWR, 0666)) == -1)
    {
        perror("shm_open");
        exit(1);
    }
    // Map shared memory
    if ((shm = mmap(NULL, SHMSIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0)) == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }
    // Truncate shared memory
    if (ftruncate(shmid, SHMSIZE) == -1)
    {
        perror("ftruncate");
        exit(1);
    }
    pid_t pid;
    int pipefd[2];

    // Create a pipe
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(1);
    }

    // Create first child process
    pid = fork();
    if (pid == 0)
    {
        // this is Child process retrieving input
        char buf[BUFSIZE];
        int len = 0;
        int fd = open("sample_in_process.txt", O_RDONLY);
        if (fd == -1)
        {
            perror("open");
            exit(1);
        }

        while ((len = read(fd, buf, BUFSIZE)) > 0)
        {
            strncpy(shm, buf, len);
            shm[len] = '\0';
        }
        close(fd);
        exit(0);
    }
    else if (pid > 0)
    {
        // Parent process
        wait(NULL);
        pid_t pid2;

        // create second child process
        pid2 = fork();
        if (pid2 == 0)
        {
            // Child process
            int fd = open("sample_in_process.txt", O_RDONLY);
            if (fd == -1)
            {
                perror("open");
                exit(1);
            }
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);

            // Executing shell commands
            char *args[] = {"sh", "-c", shm, NULL};
            execvp(args[0], args);
            exit(0);
        }
        else if (pid2 > 0)
        {
            // Parent process
            wait(NULL);

            // write to output file
            close(pipefd[1]);
            char buf[BUFSIZE];
            int len = 0;
            while ((len = read(pipefd[0], buf, BUFSIZE)) > 0)
            {
                writeOutput(shm, buf);
            }
        }
    }
    return 0;
}