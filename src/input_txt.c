#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//function για να μετρήσουμε τις γραμμές ενος αρχείου
int count_lines(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    int lines = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        lines++;
    }

    fclose(file);
    return lines;
}


//function για να απελευθερώσουμε την μνήμη που δεσμεύσαμε για τις γραμμές
void free_lines(char **lines, int line_count) {
    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }
    free(lines);
}
