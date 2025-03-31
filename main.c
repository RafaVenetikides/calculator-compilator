//
// Created by Rafael Venetikides on 21/03/25.
// Grupo: RA1 7
//

#include <stdio.h>
#include <stdlib.h>
#include "include/file_utils.h"
#include "include/expression_parser.h"

int main() {
    FILE *fptr = fopen("file_2.txt", "r");
    if (!fptr) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    FILE *outAsm = fopen("assembly_output.asm", "w");
    if (!outAsm) {
        perror("Erro ao criar arquivo de saída");
        fclose(fptr);
        return 1;
    }

    char **lines = NULL;
    size_t count = 0;

    read_lines(&lines, &count, fptr);
    fclose(fptr);

    for (size_t i = 0; i < count; i++) {
        process_line(lines[i], outAsm);
        free(lines[i]);
    }
    free(lines);

    return 0;
}