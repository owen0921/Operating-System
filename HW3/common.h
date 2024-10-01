#ifndef COMMON_H
#define COMMON_H

#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define MMAP_MEM_SIZE 1024 // Define the size of the shared memory

typedef struct {
    int locked;               // Mutex lock, 0 means unlocked, 1 means locked
    int ready;                // To check if the shared memory can be read or not, 0 means not ready, 1 means ready
    char data[MMAP_MEM_SIZE]; // Shared memory data
}shared_memory_t;

void init_shared_memory(shared_memory_t **shm);     // Initialize the shared memory
void destroy_shared_memory(shared_memory_t *shm);   // Destroy the shared memory

#endif