#include "../headers/shm.h"
#include <stdio.h>
#include <string.h>

// Δημιουργία shared memory
int shm_create(key_t key, size_t size) {
    int shm_id = shmget(key, size, 0666 | IPC_CREAT);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    return shm_id;
}

// Διαγραφή shared memory
void shm_delete(int shm_id) {
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID failed");
        exit(EXIT_FAILURE);
    }
}

// Σύνδεση με shared memory
SharedData* shm_attach(int shm_id) {
    SharedData* shm_ptr = (SharedData*)shmat(shm_id, NULL, 0);
    if (shm_ptr == (void*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    return shm_ptr;
}

// Αποσύνδεση από shared memory
void shm_detach(SharedData* shm_ptr) {
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }
}


// int main() {
//     key_t key = ftok("shmfile", 65);
//     size_t size = sizeof(SharedData);
//     int shm_id = shm_create(key, size);
//     SharedData* shm_ptr = shm_attach(shm_id);

//     strcpy(shm_ptr->message, "Hello from shared memory");
//     printf("Message: %s\n", shm_ptr->message);
    
//     shm_detach(shm_ptr);
//     shm_delete(shm_id);

//     return 0;
// }
