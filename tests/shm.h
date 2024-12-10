#ifndef SHM_H
#define SHM_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>

// Define shared memory structure
typedef struct data {
    char message[1024]; // Message string
} SharedData;

// Functions for shared memory
int shm_create(key_t key, size_t size);
void shm_delete(int shm_id);
SharedData* shm_attach(int shm_id);
void shm_detach(SharedData* shm_ptr);

#endif
