//
// Created by Rafael Venetikides on 21/03/25.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../include/expression_parser.h"

#include <stdlib.h>

#include "../include/operations.h"

void process_line(const char *line, FILE *outAsm) {
    char postfix[256] = "";
    int pos = 0;

    char* out = parse_expression(line, &pos, postfix);
    generateAssemblyFromPostfix(out, outAsm);

    printf("Assembly generated\n", out);
}

char* parse_expression(const char *line, int *pos, char *out) {
    // out é a string onde vamos acumulando tokens em notação pós-fixa
    skip_spaces(line, pos);

    // Esperamos encontrar '(' para iniciar a subexpressão
    if (line[*pos] != '(') {
        return out; // não é subexpressão => nada a fazer
    }
    (*pos)++; // pula '('
    skip_spaces(line, pos);

    // ---------- 1) Lê primeiro operando ----------
    if (line[*pos] == '(') {
        // subexpressão aninhada, chama recursivo
        parse_expression(line, pos, out);
    } else {
        // token simples (ex.: "7")
        int start = *pos;
        while (line[*pos] && !isspace((unsigned char)line[*pos])
               && line[*pos] != ')' && line[*pos] != '(') {
            (*pos)++;
               }
        int length = *pos - start;
        strncat(out, line + start, length);
        strcat(out, " "); // separador
    }

    skip_spaces(line, pos);

    // ---------- 2) Lê segundo operando ----------
    if (line[*pos] == '(') {
        parse_expression(line, pos, out);
    } else {
        int start = *pos;
        while (line[*pos] && !isspace((unsigned char)line[*pos])
               && line[*pos] != ')' && line[*pos] != '(') {
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