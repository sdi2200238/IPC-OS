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

char **read_lines(const char *filename, int *line_count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    *line_count = count_lines(filename);
    char **lines = malloc(*line_count * sizeof(char *));
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

        size_t length = strlen(buffer);
        if (buffer[length - 1] == '\n') {
            buffer[length - 1] = '\0'; // Remove newline character
        }

        lines[i] = strdup(buffer);
        if (!lines[i]) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
    return lines;
}

//function to free the memory allocated for the array of strings
void free_lines(char **lines, int line_count) {
    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }
    free(lines);
}
