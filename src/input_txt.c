//file to read text file into an array of strings
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


//function to read the lines of the text file into an array of strings
int read_lines(const char *filename, char **lines, int *line_count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    *line_count = count_lines(filename);
    lines = (char **)malloc(*line_count * sizeof(char *));
    if (!lines) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    for (int i = 0; i < *line_count; i++) {
        if (!fgets(buffer, sizeof(buffer), file)) {
            perror("Failed to read line");
            exit(EXIT_FAILURE);
        }
        lines[i] = strdup(buffer);
    }
    
    fclose(file);
    return 0;
}

//function to free the memory allocated for the array of strings
void free_lines(char **lines, int line_count) {
    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }
    free(lines);
}
