//
// Created by Rafael Venetikides on 03/04/25.
//

#include "../../include/operations/subtract_operation.h"


void op_sub_16bits(FILE *outAsm) {

    fprintf(outAsm, "op_sub_16bits:\n\n"
                                    "   PUSH R19\n"
                                    "   PUSH R18\n"
                                    "   PUSH R17\n"
                                    "   PUSH R16\n"
                                    "   PUSH R20\n"
                                    "   PUSH R21\n"
                                    "   PUSH R24\n"
                                    "   PUSH R25\n"
                                    "\n");

    fprintf(outAsm, "   ORI R19, 0x80\n\n"
                                   "   CALL op_add_16bits\n\n");

                    fprintf(outAsm, "   POP R25\n"
                                    "   POP R24\n"
                                    "   POP R21\n"
                                    "   POP R20\n"
                                    "   POP R16\n"
                                   "   POP R17\n"
                                   "   POP R18\n"
                                   "   POP R19\n");

    fprintf(outAsm, "   RET\n\n");
}
