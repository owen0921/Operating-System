#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#define MMAP_MEM_SIZE 1024 // Define the size of the shared memory
#define NUM_READERS 2      // Number of readers

typedef struct {
    int locked;               // Mutex lock, 0 means unlocked, 1 means locked
    int ready;                // To check if the shared memory can be read or not, 0 means not ready, 1 means ready
    char data[MMAP_MEM_SIZE]; // Shared memory data
} shared_memory_t;

// Function declarations
void init_shared_memory(shared_memory_t **shm);
void destroy_shared_memory(shared_memory_t *shm);
void reader(int id, shared_memory_t *shm);
void writer(shared_memory_t *shm);
char* Getchar(char* str);

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

            } else if (pid < 0) {
                perror("Fork error!\n");
                exit(-1);
            } else {
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

        // Reset ready flag
        shm->ready = 0;
    }

    // Destroy the shared memory
    destroy_shared_memory(shm);
    return 0;
}

void reader(int id, shared_memory_t *shm) {
    // Wait until data is ready
    while (!shm->ready);

    // Acquire lock (spinlock)
    while (__sync_lock_test_and_set(&shm->locked, 1) != 0);

    printf("Process %d named %d launching as reader\n", id, getpid());

    printf("Process %d received: ", getpid());

    // Use putchar to print out the data which is in the shared memory
    char *data_ptr = shm->data;
    while (*data_ptr != '\0') {
        putchar(*data_ptr++);
    }
    putchar('\n');  // Use '\n' instead of "\n"

    // Notify that the process has finished reading
    printf("Process %d has read the data from the shared memory!\n", getpid());

    // Release lock
    __sync_lock_release(&shm->locked);
}

// Initialize the shared memory
void init_shared_memory(shared_memory_t **shm) {
    /* PROT_READ | PROT_WRITE : indicate that the shared memory can be read or write 
     * MAP_SHARED : Multiple processes can share this memory area,
     *              and modifications to this memory are visible to all processes.
     * MAP_ANON : This memory is not associated with any file, it only exists in the memory
     */
    *shm = (shared_memory_t *)mmap(NULL, sizeof(shared_memory_t), 
    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    
    if (*shm == (void *)-1) {
        perror("Fail to mmap!\n");
        exit(-1);
    }

    (*shm)->locked = 0; // The shared memory is unlocked
    (*shm)->ready = 0;  // The shared memory is not ready to read
}

// Destroy the shared memory
void destroy_shared_memory(shared_memory_t *shm) {
    munmap(shm, sizeof(shared_memory_t));
}

char* Getchar(char* str) {
    // Allocate enough memory to store the input string
    char* input_str = (char*)malloc(32 * sizeof(char));
    if (input_str == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Prompt the user to enter a string
    printf("Enter a string, control + D to exit: ");
    if (scanf("%s", input_str) == EOF) {
        free(input_str);
        return NULL;
    }

    // Deal with the situation that the input string is more than 32 characters
    if (strlen(input_str) > 32) {
        input_str[32] = '\0';
    }

    // Copy the input string to the target string
    strcpy(str, input_str);

    // Free dynamically allocated memory
    free(input_str);

    return str;
}

void writer(shared_memory_t *shm) {
    // Print which process is the writer
    printf("Process %d launching as writer\n", getpid());

    // Acquire lock (spinlock)
    while (__sync_lock_test_and_set(&shm->locked, 1) != 0);

    // Read user input using Getchar()
    char *data_ptr = shm->data;
    if (Getchar(data_ptr) == NULL) {
        // If EOF is detected, exit the program
        shm->ready = 0;
        __sync_lock_release(&shm->locked);
        printf("EOF detected!\n");
        exit(0);
    }

    printf("Parent process has written to shared memory: %s\n", shm->data);

    // Signal that data is ready
    shm->ready = 1;

    // Release lock
    __sync_lock_release(&shm->locked);
}
