//
// Created by Rafael Venetikides on 25/03/25.
//

#ifndef STACK_OPERATIONS_H
#define STACK_OPERATIONS_H

#include <stdio.h>
#include <stdint.h>
#include "float_conversions.h"

void gen_push_16bit(float val, FILE *outAsm);
void gen_pop_16bit(FILE *outAsm);

#endif //STACK_OPERATIONS_H
