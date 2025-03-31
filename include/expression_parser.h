//
// Created by Rafael Venetikides on 21/03/25.
//

#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

void process_line(const char *line, FILE *outAsm);
char* parse_expression(const char *line, int *pos, char *out);
static void skip_spaces(const char *line, int *pos);
void generateAssemblyFromPostfix(const char *postfix, FILE *outAsm);

#endif //EXPRESSION_PARSER_H
