#include "common.h"

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
