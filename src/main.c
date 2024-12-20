#include "../headers/main.h"

//function για να κανουμε spawn ενα παιδί
void spawn_child(int sem_id[], int shm_id, pid_t *children, int index); //function για να τερματίσουμε ενα παιδί
void terminate_child(pid_t child_pid, Stats stats);

int max_semaphores;
int main(int argc, char *argv[]) {
    // Έλεγχος ορισμάτων
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <command_file> <text_file> <max_children> <max_semaphores>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *command_file = argv[1];
    const char *text_file = argv[2];
    MAX_CHILDREN = atoi(argv[3]);
    max_semaphores = atoi(argv[4]);

    // Δημιουργία κλειδιών με βάση τα ονόματα των αρχείων
    key_t sem_key = ftok(command_file, 'S');
    key_t shm_key = ftok(command_file, 'M');
    // Δημιουργία shared memory και semaphores
    int sem_id[max_semaphores];
    int shm_id = shm_create(shm_key, sizeof(SharedData));

    SharedData *shm_ptr = shm_attach(shm_id);

    for (int i = 0; i < max_semaphores; i++) {
        sem_id[i] = sem_create(sem_key + i);
        set_sem(sem_id[i]);
    }

    // Άνοιγμα αρχείων
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


    //Αρχικοποίηση μεταβλητών
    pid_t *children = (pid_t *)calloc(MAX_CHILDREN, sizeof(pid_t));
    Stats *stats = (Stats *)calloc(MAX_CHILDREN, sizeof(Stats));
    int active_children = 0;
    char line[256];
    
    srand(time(NULL));
    
    // Διάβασμα γραμμών από το text file σε ενα array
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


    int current_time = 0;
    int timestamp;
    char label[10];
    char command;
    fgets(line, sizeof(line), cf);
    sscanf(line, "%d %s %c", &timestamp, label, &command);
    // Κύκλος εκτέλεσης
    while (1) {
        printf("Cycle: %d\n", current_time);

        // Ελεγχος αν έχει φτάσει στο timestamp της επομενης εντολής
        if (timestamp == current_time) {
            // Spawn ενα παιδί
            if (command == 'S' && active_children < MAX_CHILDREN) {
                int child_index = atoi(label + 1) - 1;
                spawn_child(sem_id, shm_id, children, child_index);
                printf("Spawned child %d\n", children[child_index]);
                stats[child_index].start_time = current_time;
                active_children++;
            } 
            // Τerminate ενα παιδί
            else if (command == 'T') { 
                int child_index = atoi(label + 1) - 1;
                if (children[child_index] == 0) { // Έλεγχος αν το παιδί υπάρχει
                    fprintf(stderr, "Child has not been spawned, can't terminate\n");
                } else {
                    stats[child_index].end_time = current_time;
                    terminate_child(children[child_index], stats[child_index]);
                    children[child_index] = 0;
                    active_children--;
                }
            }
            
            if (!fgets(line, sizeof(line), cf)) {
                label[0] = 'E';
            }
            // Επόμενο command
            sscanf(line, "%d %s %c", &timestamp, label, &command);
        }        
        // Έλεγχος αν υπάρχουν active παιδιά
        
        if (active_children > 0) {
            int random_child_index = rand() % MAX_CHILDREN;
            // Εύρεση τυχαίου active παιδιού
            while (children[random_child_index] == 0) {
                random_child_index = (random_child_index + 1) % MAX_CHILDREN;  
            }

            int semaphore_index = random_child_index % max_semaphores;  //Δωσε στο παιδί ενα semaphore κυκλικα

            // Αποστολή μηνύματος στο τυχαίο παιδί
            int random_line_index = rand() % line_count;
            // Κοινοποίηση μηνύματος στο shared memory
            strcpy(shm_ptr->message, lines[random_line_index]);
            shm_ptr->child_index = random_child_index;
            printf("Parent sent to child %d: %s\n", random_child_index + 1, shm_ptr->message);

            stats[random_child_index].count++;  

            sem_up(sem_id[semaphore_index]);  // Ενημέρωση του παιδιού
            sem_down(sem_id[semaphore_index]); // Αναμονή του παιδιού
        }
        
        else {
            printf("No active children\n");
        }

        // Αύξηση του χρόνου/cycle
        current_time++;
        sleep(0.1);
        // Έλεγχος αν έχει φτάσει στο EXIT
        if (strcmp(label, "EXIT") == 0 && current_time >= timestamp) {
            break;
        }
    }

    // Τερματισμός όλων των remaining παιδιών
    for (int i = 0; i < MAX_CHILDREN; i++) {
    if (children[i] > 0) {
        int semaphore_index = i % max_semaphores;
        stats[i].end_time = current_time;
        sem_up(sem_id[semaphore_index]); // Ενημέρωση του παιδιού
        terminate_child(children[i], stats[i]);
    }
}


    // Αποδέσμευση μνήμης και κλεισιμο αρχείων
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
    int semaphore_index = index % max_semaphores; // Δωσε στο παιδί το semaphore κυκλικα

    pid_t pid = fork();
    if (pid == 0) {
        SharedData *shm_ptr = shm_attach(shm_id);  // Attach shared memory
        while (1) { 
            sem_down(sem_id[semaphore_index]);  // Περίμενε για μήνυμα από τον γονέα
            if (shm_ptr->child_index != index || strcmp(shm_ptr->message, "") == 0) {
                sem_up(sem_id[semaphore_index]);  // Signal back to parent
                continue;  // Προσπέρασε το μήνυμα
            }
            printf("Child [%d] received: %s\n", getpid(), shm_ptr->message);
            //nullify message
            strcpy(shm_ptr->message, "");
            sem_up(sem_id[semaphore_index]);  // Ενημέρωση του γονέα
        }

        shm_detach(shm_ptr); // Detach shared memory
        exit(0);
    } else if (pid > 0) {
        children[index] = pid;  // Αποθήκευση του pid του παιδιού
    }
}

void terminate_child(pid_t child_pid, Stats stats) {
    if (child_pid > 0) {
        // Αποστολή σήματος τερματισμού
        kill(child_pid, SIGTERM);
        waitpid(child_pid, NULL, 0);
        // Εκτύπωση στατιστικών παιδιού
        printf("Terminating child %d (active for %ld steps, processed %d messages)\n", child_pid, stats.end_time - stats.start_time, stats.count);
    }
}
