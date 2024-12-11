#include "../headers/sem.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
 
// Δημιουργία semaphore
int sem_create(key_t key) {
    // Δημιουργία semaphore
    int sem_id = semget(key, 1, 0666 | IPC_CREAT);
    // Έλεγχος αν η δημιουργία του semaphore ήταν επιτυχής
    if (sem_id == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }
    return sem_id;
}

// Αρχικοποίηση semaphore
int set_sem(int sem_id) {
    union semun sem_union;
    sem_union.val = 0; // Αρχικοποίηση του semaphore σε 0
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
        perror("semctl SETVAL failed");
        return 0;
    }
    return 1;
}

// Διαγραφή semaphore
void sem_del(int sem_id) {
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1) {
        perror("semctl IPC_RMID failed");
        exit(EXIT_FAILURE);
    }
}

// Μείωση του semaphore
int sem_down(int sem_id) {
    struct sembuf sem_b = {0, -1, SEM_UNDO};
    if (semop(sem_id, &sem_b, 1) == -1) {
        perror("sem_down failed");
        return 0;
    }
    return 1;
}

// Αύξηση του semaphore
int sem_up(int sem_id) {
    struct sembuf sem_b = {0, 1, SEM_UNDO};
    if (semop(sem_id, &sem_b, 1) == -1) {
        perror("sem_up failed");
        return 0;
    }
    return 1;
}
