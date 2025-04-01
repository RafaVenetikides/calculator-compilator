//
// Created by Rafael Venetikides on 24/03/25.
//

#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdio.h>

void generateAssemblyFromPostfix(const char *postfix, FILE *outAsm);
void write_functions(FILE *outAsm);
void op_add_16bits(FILE *outAsm);
void align_exp(FILE *outAsm);
void op_sub_16bits(FILE *outAsm);
void op_mult_16bits(FILE *outAsm);
void op_real_divide(FILE *outAsm);
void op_integer_divide(FILE *outAsm);
void op_modulo(FILE *outAsm);
void op_power(FILE *outAsm);

#endif //OPERATIONS_H
