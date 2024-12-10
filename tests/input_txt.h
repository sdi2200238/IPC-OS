//header for input
#ifndef INPUT_TXT_H
#define INPUT_TXT_H

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

int count_lines(const char *filename);
char **read_lines(const char *filename, int *line_count);
void free_lines(char **lines, int line_count);

#endif
