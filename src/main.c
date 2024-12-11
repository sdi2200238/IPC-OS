#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "../headers/sem.h"
#include "../headers/shm.h"



typedef struct {
    time_t start_time;
    time_t end_time;
    int count;
}Stats;

void spawn_child(int sem_id[], int shm_id, pid_t *children, int index);
void terminate_child(pid_t child_pid, Stats stats);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <command_file> <text_file> <max_semaphores>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *command_file = argv[1];
    const char *text_file = argv[2];
    int MAX_CHILDREN = atoi(argv[3]);  // Maximum number of children
    int max_semaphores = 2 * MAX_CHILDREN; // Maximum number of semaphores

    // Create semaphore and shared memory
    key_t sem_key = ftok(command_file, 'S');
    key_t shm_key = ftok(command_file, 'M');
    int sem_id[max_semaphores];  // Array of semaphores
    int shm_id = shm_create(shm_key, sizeof(SharedData));

    SharedData *shm_ptr = shm_attach(shm_id);

    // Initialize semaphores
    for (int i = 0; i < max_semaphores; i++) {
        sem_id[i] = sem_create(sem_key + i);  // Create a semaphore for each child
        set_sem(sem_id[i]);  // Set the semaphore value to 1 (ready state)
    }

    FILE *cf = fopen(command_file, "r");
    if (!cf) {
        perror("Failed to open command file");
        sem_del(sem_id);
        shm_delete(shm_id);
        exit(EXIT_FAILURE);
    }

    FILE *tf = fopen(text_file, "r");
    if (!tf) {
        perror("Failed to open text file");
        fclose(cf);
        sem_del(sem_id);
        shm_delete(shm_id);
        exit(EXIT_FAILURE);
    }

    pid_t *children = (pid_t *)calloc(MAX_CHILDREN, sizeof(pid_t));
    Stats *stats = (Stats *)calloc(MAX_CHILDREN, sizeof(Stats));
    int active_children = 0;
    char line[256];
    
    srand(time(NULL)); // Seed random number generator
    
    //read text file
    int line_count;
    char **lines;
    line_count = count_lines(text_file);
    lines = (char **)malloc(line_count * sizeof(char *));
    if (!lines) {
        perror("Failed to allocate memory");
        fclose(cf);
        fclose(tf);
        sem_del(sem_id);
        shm_delete(shm_id);
        exit(EXIT_FAILURE);
    }  
    for (int i = 0; i < line_count; i++) {
        lines[i] = (char *)malloc(1024 * sizeof(char));
        if (!lines[i]) {
            perror("Failed to allocate memory");
            free_lines(lines, i);
            fclose(cf);
            fclose(tf);
            sem_del(sem_id);
            shm_delete(shm_id);
            exit(EXIT_FAILURE);
        }
        fgets(lines[i], 1024, tf);
    }


    int current_time = 0; // Time counter for the parent process
    int timestamp;
    char label[10];
    char command;
    fgets(line, sizeof(line), cf);
    sscanf(line, "%d %s %c", &timestamp, label, &command);
    printf("Time: %d\n", current_time);
while (1) {
    // Check if there is a command at the current timestamp
    printf("Time: %d\n", current_time);

    if (timestamp == current_time) {

        // Execute commands based on the timestamp
        if (command == 'S' && active_children < MAX_CHILDREN) {
            int child_index = atoi(label + 1) - 1;
            spawn_child(sem_id, shm_id, children, child_index);
            active_children++;
        } else if (command == 'T') {
            int child_index = atoi(label + 1) - 1;
            if (children[child_index] == 0) {
                fprintf(stderr, "Child has not been spawned, can't terminate\n");
            } else {
                stats[child_index].end_time = current_time;
                terminate_child(children[child_index], stats[child_index]);
                children[child_index] = 0;
                active_children--;
            }
        }
        

        // Read the next line for future commands
        if (!fgets(line, sizeof(line), cf)) {
            label[0] = 'E'; // Mark end of commands if EOF is reached
        }
        sscanf(line, "%d %s %c", &timestamp, label, &command);
    }

    // Always send a random line to an active child, regardless of command execution
    if (active_children > 0) {
        int random_child_index = rand() % active_children;
        while (children[random_child_index] == 0) { // Pick active child
            random_child_index = (random_child_index + 1) % MAX_CHILDREN;
        }

        // Read a random line from the text file
        int random_line_index = rand() % line_count;
        strcpy(shm_ptr->message, lines[random_line_index]);
        printf("Parent sent to child %d: %s\n", random_child_index + 1, shm_ptr->message);
        stats[random_child_index].count++;
        sem_up(sem_id[random_child_index]); // Signal the selected child

        // Wait for the child to finish processing the message
        sem_down(sem_id[random_child_index]);
    }

    current_time++;
    
    
    if (strcmp(label, "EXIT") == 0 && current_time >= timestamp) {
        break; // Exit if end of commands is reached
    }
}

    // Cleanup
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (children[i] > 0) {
            stats[i].end_time = current_time;
            terminate_child(children[i], stats[i]);
        }
    }

    shm_detach(shm_ptr);
    shm_delete(shm_id);
    for (int i = 0; i < max_semaphores; i++) {
        sem_del(sem_id[i]);
    }

    free_lines(lines, line_count);
    fclose(cf);
    fclose(tf);
    free(children);
    free(stats);
    return 0;
}

void spawn_child(int sem_id[], int shm_id, pid_t *children, int index) {
    pid_t pid = fork();
    if (pid == 0) {
        SharedData *shm_ptr = shm_attach(shm_id);
        while (1) {
            sem_down(sem_id[index]); // Wait for the parent to send data
            printf("Child [%d] received: %s\n", getpid(), shm_ptr->message);
            sleep(1); // Simulate processing delay
            sem_up(sem_id[index]); // Signal the parent that we are ready for the next line
        }
        shm_detach(shm_ptr);
        exit(0);
    } else if (pid > 0) {
        children[index] = pid;
    }
}


void terminate_child(pid_t child_pid, Stats stats) {
    if (child_pid > 0) {
        kill(child_pid, SIGTERM);
        waitpid(child_pid, NULL, 0);
        printf("Terminating child %d at %ld with %d messages\n\n", child_pid, stats.end_time, stats.count);
    }
}
