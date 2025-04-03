//
// Created by Rafael Venetikides on 24/03/25.
//

#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdio.h>
#include "../include/operations/add_operation.h"
#include "../include/operations/subtract_operation.h"
#include "../include/operations/multiply_operation.h"
#include "../include/operations//div_operation.h"
#include "../include/operations/div_int_operation.h"
#include "../include/operations/rem_operation.h"
#include "../include/operations/power_operation.h"

void generateAssemblyFromPostfix(const char *postfix, FILE *outAsm);
void write_functions(FILE *outAsm);
void align_exp(FILE *outAsm);

#endif //OPERATIONS_H
