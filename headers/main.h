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


//struct για να αποθηκεύουμε τα στατιστικά των παιδιών
typedef struct {
    time_t start_time;
    time_t end_time;
    int count;
} Stats;

int MAX_CHILDREN;