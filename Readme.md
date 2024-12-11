# Εργασία Ι Λειτουργικών Συστημάτων
## Στοιχεία

- **Oνοματεπώνυμο**: Κορνήλιος Ιωάννου
- **Email**: sdi2200238@di.uoa.gr
- **Α.Μ.**: 1115202200238

## Πληροφορίες Εργασίας

- **Εργασία**: IPC-OS
- **Description**: Υλοποίηση ενος συστήματος Inter-Process Communication (IPC) με χρήση κοινής μνήμης και semaphores σε γλώσσα C.
- **Μάθημα**: Λειτουργικά Συστήματα
- **Ημερομηνία**: Δεκέμβριος 2024

## Εκτέλεση

1. **Μεταγλώττιση**: `make`
2. **Εκτέλεση**: `./ipc_os <command_file> <text_file> <max_semaphores>`
3. **Καθαρισμός**: `make clean`

---

### **Documentation Εργασίας**

### **1. Introduction**

Αυτό το αρχείο αναλύει την διαδικασία υλοποίηση ενός συστήματος IPC που χρησιμοποιεί shared memory και semaphores για τον συγχρονισμό. Το project περιλαμβάνει τη δημιουργία ενός parent process («P») και πολλαπλών child processes («Ci»), όπου ο γονέας και τα παιδιά ανταλλάσσουν μηνύματα. Ο γονέας στέλνει τυχαίες γραμμές από ένα αρχείο κειμένου στα παιδιά και τα παιδιά επεξεργάζονται τα μηνύματα και απαντούν ανάλογα.

### **2. Διαδικασία Ανάπτυξης**

#### **2.1 Σχεδιασμός και Οργάνωση**

Χώρισα το project στα ακόλουθα parts:

1. **Υλοποίηση shared memory** 

2. **Υλοποίηση semaphores** 

3. **Υλοποίηση parent process**

4. **Υλοποίηση child process**

#### **2.2 Περιβάλλον και Χαρακτηριστικά**

- **Γλώσσα Προγραμματισμού:** C 
- **IPC Mechanisms:** Shared memory (shmget, shmat) and semaphores (semget, semctl, semop).
- **Compiler:** GCC
- **Operating System:** Linux/Unix (WSL) 
- **Editor:** Visual Studio Code

### **3. Λεπτομέριες υλοποίησης**

#### **3.1 Shared Memory**

Η δομή `SharedData` ορίζεται ως εξής:

```c
typedef struct {
    char message[1024]; // Buffer for messages
    int number;         // Placeholder for additional data
} SharedData;
```

Δημιουργήσα την κοινή μνήμη με την χρήση της `shmget` για την απόκτηση ενός τμήματος κοινής μνήμης, `shmat` για την προσάρτηση της κοινής μνήμης στη διεύθυνση χώρου της διεργασίας, και `shmdt` για την αποσύνδεση της κοινής μνήμης όταν δεν χρειάζεται πλέον.

Έτσι υλοποιήσα τις συναρτήσεις: 
- `shm_create` : Δημιουργεί και αρχικοποιεί την κοινή μνήμη.
- `shm_delete` : Διαγράφει την κοινή μνήμη και αποδεσμεύει το memory.
- `shm_attach` : Κανει attach τη κοινή μνήμη με τη διεύθυνση χώρου της διεργασίας.
- `shm_detach` : Αποσύνδεση της κοινής μνήμης όταν δεν χρειάζεται πλέον.
```c
int shm_create(key_t key, size_t size);
void shm_delete(int shm_id);
SharedData* shm_attach(int shm_id);
void shm_detach(SharedData* shm_ptr);
```

Για το testing των συναρτήσεων υλοποιησα μια sample `main`.

```c
int main() {
  key_t key = ftok("shmfile", 65);
  size_t size = sizeof(SharedData);
  int shm_id = shm_create(key, size);
  SharedData* shm_ptr = shm_attach(shm_id);

  strcpy(shm_ptr->message, "Hello from shared memory");
  printf("Message: %s\n", shm_ptr->message);
  
  shm_detach(shm_ptr);
  shm_delete(shm_id);

  return 0;
}
```

#### **3.2 Semaphores**

Η υλοποίηση έγινε με χρήση των `System V` semaphores.

Aρχικά, όρισα τη δομή `semun` ως εξής:

```c
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};
```
Κάθε παιδί έχει δικό του semaphore. Tα semaphores θα:

- Μπλοκάρουν το παιδί μέχρι να λάβει μύνημα απο τον γονέα.
- Eνημερώσουν τον γονέα οταν το παιδί έχει κάνει process το παιδί.

Έτσι αποφεύγουμε τα race conditions.

Δημιουργήσα την υλοποίηση των semaphores με την χρήση `semget` για τη δημιουργία ενός συνόλου σημαφόρων, `semctl` για τον έλεγχο και τη διαχείριση των σημαφόρων, και `semop` για την εκτέλεση λειτουργιών σε σημαφόρους.

Έτσι υλοποιήσα τις συναρτήσεις: 
- `sem_create` : Δημιουργεί ενα semaphore.
- `set_sem`    : Ορίζει την τιμή ενος semaphore σε 0.
- `sem_del`    : Διαγράφει ενα semaphore και αποδεσμεύει την μνήμη του.
- `shm_detach` : Αποσύνδεση της κοινής μνήμης όταν δεν χρειάζεται πλέον.

```c
int sem_create(key_t key);
int set_sem(int sem_id);
void sem_del(int sem_id);
int sem_down(int sem_id);
int sem_up(int sem_id);
```

Για το testing των συναρτήσεων υλοποιησα μια sample `main`.

```c
int main() {
  key_t key = ftok("semfile", 65);
  int sem_id = sem_create(key);
  set_sem(sem_id);
  sem_up(sem_id);
  sem_down(sem_id);
  sem_del(sem_id);
  return 0;
}
```

#### **3.3 Parent Process**

Το parent process (`main.c`) λειτουργεί ως εξής:
1. Διαβάζει από ένα αρχείο εντολών (`CF`) μια γραμμή που περιέχει ένα timestamp, μια από τις ακόλουθες εντολές: `S` (spawn), `T` (terminate), και ένα label που περιγράφει για ποιο παιδί αναφέρεται.
2. Ανοίγει ένα αρχείο κειμένου (`TF`) και διαβάζει όλες τις γραμμές σε ένα array.
3. Δημιουργεί shared memory και semaphores για την επικοινωνία με τα παιδιά.
4. Σε κάθε κύκλο εκτέλεσης:
  - Ελέγχει αν έχει φτάσει στο timestamp της επόμενης εντολής.
    - Αν η εντολή είναι `S`, δημιουργεί ένα νέο παιδί και το προσθέτει στη λίστα των ενεργών παιδιών.
    - Αν η εντολή είναι `T`, τερματίζει το αντίστοιχο παιδί και ενημερώνει τα στατιστικά του.
  - Επιλέγει τυχαία ένα ενεργό παιδί και του στέλνει μια τυχαία γραμμή από το αρχείο κειμένου μέσω της shared memory.
  - Περιμένει το παιδί να επιβεβαιώσει την παραλαβή του μηνύματος μέσω του semaphore.
5. Τερματίζει όλα τα ενεργά παιδιά και αποδεσμεύει τους πόρους (shared memory, semaphores) πριν τερματίσει.


#### **3.4 Child Processes**

Το child process λειτουργεί ως εξής:
1. Εκτελείται σε έναν άπειρο βρόχο, περιμένοντας ένα σήμα semaphore από τον γονέα.
2. Διαβάζει το μήνυμα από την κοινή μνήμη και το επεξεργάζεται (π.χ., το εκτυπώνει στην κονσόλα).
3. Σηματοδοτεί τον γονέα όταν είναι έτοιμο για το επόμενο μήνυμα.
4. Τερματίζει ομαλά όταν λάβει σήμα τερματισμού (`kill`).


Η διαχείρηση των παιδιών που περιγράφεται παραπάνω γίνεται με τις εξής συναρτήσεις:
```c
void spawn_child(int sem_id[], int shm_id, pid_t *children, int index);
void terminate_child(pid_t child_pid, Stats stats);
```


### **4. Build and Run**

Για να κατασκευάσετε και να εκτελέσετε το πρόγραμμα έκανα include και ένα `Makefile`.

1. **Μεταγλώττιση**: Χρησιμοποιήστε την εντολή `make` για δημιουργήσεται το executable.
  ```sh
  make
  ```

2. **Εκτέλεση**: Εκτελέστε το πρόγραμμα με τα απαραίτητα αρχεία εντολών και κειμένου, καθώς και τον μέγιστο αριθμό παιδιών.
  ```sh
  ./build/ipc_os <command_file> <text_file> <max_children>

  example:

  ./build/ipc_os ./files/config_10_10000.txt ./files/mobydick.txt 15
  ```

3. **Καθαρισμός**: Χρησιμοποιήστε την εντολή `make clean` για να καθαρίσετε τα παραγόμενα αρχεία.
  ```sh
  make clean
  ```