//
// Created by Rafael Venetikides on 03/04/25.
//

#include "../../include/operations/subtract_operation.h"


void op_sub_16bits(FILE *outAsm) {

    fprintf(outAsm, "op_sub_16bits:\n\n");

    fprintf(outAsm, "   ORI R19, 0x80\n\n");

    fprintf(outAsm, "   CALL op_add_16bits\n\n");

    fprintf(outAsm, "   RET\n\n");
}
