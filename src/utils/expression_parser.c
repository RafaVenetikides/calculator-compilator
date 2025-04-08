//
// Created by Rafael Venetikides on 21/03/25.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../../include/utils/expression_parser.h"

#include <stdlib.h>

#include "../../include/generate_assembly.h"

void process_line(const char *line, FILE *outAsm) {
    char postfix[256] = "";
    int pos = 0;

    char* out = parse_expression(line, &pos, postfix);
    generateAssemblyFromPostfix(out, outAsm);

    printf("Assembly generated\n");
}

char* parse_expression(const char *line, int *pos, char *out) {
    skip_spaces(line, pos);

    if (line[*pos] != '(') {
        return out;
    }
    (*pos)++;

    // ---------- 1) Lê primeiro operando ----------
    parse_operand(line, pos, out);

    // Lê segundo operando
    parse_operand(line, pos, out);

    skip_spaces(line, pos);

    // ---------- 2) Lê segundo operando ----------
    if (line[*pos] == '(') {
        parse_expression(line, pos, out);
    } else {
        int start = *pos;
        while (line[*pos] && (isdigit(line[*pos]) || line[*pos] == '.')) {
            (*pos)++;
        }
        if (start < *pos) {
            int length = *pos - start;
            strncat(out, line + start, length);
            strcat(out, " ");
        }
    }

    skip_spaces(line, pos);

    // ---------- 3) Lê o operador ----------
    if (line[*pos] && line[*pos] != ')') {
        char op = line[*pos];
        (*pos)++;
        int lenOut = (int)strlen(out);
        out[lenOut] = op;
        out[lenOut + 1] = ' ';
        out[lenOut + 2] = '\0';
    }

    skip_spaces(line, pos);

    // ---------- 4) Fecha parêntese ----------
    if (line[*pos] == ')') {
        (*pos)++;
    }

    return out;
}

static void skip_spaces(const char *line, int *pos) {
    while (isspace(line[*pos])) (*pos)++;
}

void parse_operand(const char *line, int *pos, char *out) {
    skip_spaces(line, pos);

    // Se for subexpressão
    if (line[*pos] == '(') {
        parse_expression(line, pos, out);
        return;
    }

    // Se for "MEM"
    if (strncmp(&line[*pos], "MEM", 3) == 0) {
        // Copia "MEM" para out
        strncat(out, "MEM", 3);
        strcat(out, " ");
        // Avança pos
        (*pos) += 3;
        // Retorna
        return;
    }

    // Se for um número
    if (isdigit(line[*pos]) || line[*pos] == '.') {
        int start = *pos;
        while (isdigit(line[*pos]) || line[*pos] == '.') {
            (*pos)++;
        }
        int length = *pos - start;
        strncat(out, line + start, length);
        strcat(out, " ");
        return;
    }
}