#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#include "common.h"

// Function declarations
void reader(int id, shared_memory_t *shm);
void writer(shared_memory_t *shm);

#define NUM_READERS 2  // Number of readers

int main(int argc, char **argv) {
    shared_memory_t *shm;
    init_shared_memory(&shm);

    int pipe_fds[NUM_READERS][2];   // Pipes for signaling readers
    pid_t pids[NUM_READERS];        // Array to store PIDs of reader processes

    while (1) {
        // Create pipes for each reader process
        for (int i = 0; i < NUM_READERS; ++i) {
            if (pipe(pipe_fds[i]) == -1) {
                perror("Pipe error!\n");
                exit(EXIT_FAILURE);
            }
        }

        // Write user input to shared memory
        writer(shm);

        // Create reader processes
        for (int i = 0; i < NUM_READERS; ++i) {
            pid_t pid = fork();

            if (pid == 0) {
                // Close write end of the pipe in the child process
                close(pipe_fds[i][1]);
                // Wait for signal from parent process
                char buf;
                if (read(pipe_fds[i][0], &buf, 1) == -1) {
                    perror("Read error!\n");
                    exit(EXIT_FAILURE);
                }
                reader(i + 1, shm);
                // Close read end of the pipe in the child process
                close(pipe_fds[i][0]);
                exit(0);

            } 
            else if (pid < 0) {
                perror("Fork error!\n");
                exit(-1);
            } 
            else {
                pids[i] = pid;  // Store the PID of the reader process
            }
        }

        // Signal reader processes to read the shared memory
        for (int i = 0; i < NUM_READERS; ++i) {
            if (write(pipe_fds[i][1], "r", 1) == -1) {
                perror("Write error!\n");
                exit(EXIT_FAILURE);
            }

            // Close write end of the pipe in the parent process
            close(pipe_fds[i][1]);
        }

        // Wait for all reader processes to finish
        for (int i = 0; i < NUM_READERS; ++i) {
            waitpid(pids[i], NULL, 0);
        }

        // Close all pipe read ends in the parent process
        for (int i = 0; i < NUM_READERS; ++i) {
            close(pipe_fds[i][0]);
        }

        // Adjust the output line
        printf("\n\n\n");

        // Reset ready flag
        shm->ready = 0;
    }

    // destroy the shared memory
    destroy_shared_memory(shm);
    return 0;
}
