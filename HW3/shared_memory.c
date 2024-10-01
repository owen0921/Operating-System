#include "common.h"

// initialize the shared memory
void init_shared_memory(shared_memory_t **shm) {

    /* PROT_READ | PROT_WRITE : indicate that the shared memory can be read or write 
    *  MAP_SHARED : Multiple processes can share this memory area,
                    and modifications to this memory are visible to all processes.
    *  MAP_ANON : This memory is not associated with any file, it only exists in the memory
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

// destroy the shared memory
void destroy_shared_memory(shared_memory_t *shm) {
    munmap(shm, sizeof(shared_memory_t));
}
