//
// Created by Rafael Venetikides on 21/03/25.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../include/expression_parser.h"

#include <stdlib.h>

#include "../include/generate_assembly.h"

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
    skip_spaces(line, pos);

    // ---------- 1) Lê primeiro operando ----------
    if (line[*pos] == '(') {
        parse_expression(line, pos, out);
    } else {
        int start = *pos;
        while (line[*pos] && (isdigit(line[*pos]) || line[*pos] == '.')) {
            (*pos)++;
        }
        int length = *pos - start;
        strncat(out, line + start, length);
        strcat(out, " ");
    }

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