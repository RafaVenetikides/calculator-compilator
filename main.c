//
// Created by Rafael Venetikides on 21/03/25.
// Grupo: RA1 7
//

#include <stdio.h>
#include <stdlib.h>
#include "include/utils/file_utils.h"
#include "include/utils/expression_parser.h"
#include "include/generate_assembly.h"
#include "include/utils/serial_operations.h"

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

    fprintf(outAsm, "#include \"m328Pdef.inc\"\n\n");

    memory_config(outAsm);

    write_functions(outAsm);

    serial_functions(outAsm);

    for (size_t i = 0; i < count; i++) {
        process_line(lines[i], outAsm);
        free(lines[i]);
    }
    free(lines);


    fprintf(outAsm, "end:\n"
    "    RJMP end\n");

    return 0;
}