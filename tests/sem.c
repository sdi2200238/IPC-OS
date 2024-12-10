#include "sem.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
 
//compilation: gcc -o sem sem.c -Wall
int sem_create(key_t key) {
    int sem_id = semget(key, 1, 0666 | IPC_CREAT);
    if (sem_id == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }
    return sem_id;
}

int set_sem(int sem_id) {
    union semun sem_union;
    sem_union.val = 0; // Initialize semaphore value
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
        perror("semctl SETVAL failed");
        return 0;
    }
    return 1;
}

void sem_del(int sem_id) {
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1) {
        perror("semctl IPC_RMID failed");
        exit(EXIT_FAILURE);
    }
}

int sem_down(int sem_id) {
    struct sembuf sem_b = {0, -1, SEM_UNDO};
    if (semop(sem_id, &sem_b, 1) == -1) {
        perror("sem_down failed");
        return 0;
    }
    return 1;
}

int sem_up(int sem_id) {
    struct sembuf sem_b = {0, 1, SEM_UNDO};
    if (semop(sem_id, &sem_b, 1) == -1) {
        perror("sem_up failed");
        return 0;
    }
    return 1;
}
/*
int main() {
    key_t key = ftok("semaphores.h", 65);
    int sem_id = sem_create(key);

    if (set_sem(sem_id)) {
        printf("Semaphore initialized successfully.\n");
    }

    printf("Semaphore up: %d\n", sem_up(sem_id));
    printf("Semaphore down: %d\n", sem_down(sem_id));

    sem_del(sem_id);
    return 0;
}
*/