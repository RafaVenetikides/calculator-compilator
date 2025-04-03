//
// Created by Rafael Venetikides on 21/03/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/utils/file_utils.h"

void read_lines(char ***linesPtr, size_t *countPtr, FILE *fptr) {
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), fptr)) {
        buffer[strcspn(buffer, "\n")] = '\0';

        char **temp = realloc(*linesPtr, (*countPtr + 1) * sizeof(**linesPtr));
        if (!temp) {
            perror("Erro ao realocar memória");
            exit(1);
        }
        *linesPtr = temp;

        (*linesPtr)[*countPtr] = malloc(strlen(buffer) + 1);
        if (!(*linesPtr)[*countPtr]) {
            perror("Erro ao alocar memória");
            exit(1);
        }
        strcpy((*linesPtr)[*countPtr], buffer);
        (*countPtr)++;
    }
}