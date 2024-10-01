#include "common.h"

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
    // print which process is the writer
    printf("Process %d launching as writer\n", getpid());

    // Acquire lock (spinlock)
    while (__sync_lock_test_and_set(&shm->locked, 1) != 0);

    // Read user input using Getchar()
    char *data_ptr = shm->data;
    if (Getchar(data_ptr) == NULL) {
        // If EOF is detected, exit the program
        shm->ready = 0;
        __sync_lock_release(&shm->locked);
        printf("EOF detected, exiting...\n\n\n");
        exit(0);
    }

    printf("Parent process has written to shared memory: %s\n", shm->data);

    // Signal that data is ready
    shm->ready = 1;

    // Release lock
    __sync_lock_release(&shm->locked);
}
