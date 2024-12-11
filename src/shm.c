#include "../headers/shm.h"
#include <stdio.h>
//compilation: gcc -o shm shm.c -Wall

int shm_create(key_t key, size_t size) {
    int shm_id = shmget(key, size, 0666 | IPC_CREAT);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    return shm_id;
}

void shm_delete(int shm_id) {
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID failed");
        exit(EXIT_FAILURE);
    }
}

SharedData* shm_attach(int shm_id) {
    SharedData* shm_ptr = (SharedData*)shmat(shm_id, NULL, 0);
    if (shm_ptr == (void*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    return shm_ptr;
}

void shm_detach(SharedData* shm_ptr) {
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }
}
