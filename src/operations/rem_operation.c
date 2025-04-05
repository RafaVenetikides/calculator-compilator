//
// Created by Rafael Venetikides on 03/04/25.
//

#include "../../include/operations/rem_operation.h"

void op_rem_16bits(FILE *outAsm) {
    fprintf(outAsm, "op_rem_16bits:\n"
                    "    ; Entrada: A em R19:R18, B em R17:R16\n"
                    "    ; Saída: Resto (A % B) em R23:R22\n"
                    "\n"
                    "rem_loop:\n"
                    "    ; Verifica se A (R19:R18) >= B (R17:R16)\n"
                    "    CP    R16, R18\n"
                    "    CPC   R17, R19\n"
                    "    BRLO  end_rem   ; Se A < B, termina\n"
                    "\n"
                    "    CALL  op_sub_16bits ; Resultado em R23:R22 (A - B)\n"
                    "\n"
                    "    ; Atualiza A com o resultado da subtração\n"
                    "    MOV   R17, R23\n"
                    "    MOV   R16, R22\n"
                    "\n"
                    "    RJMP  rem_loop\n"
                    "\n"
                    "end_rem:\n"
                    "    ; Quando A < B, A é o resto → retorna em R23:R22\n"
                    "    MOV   R23, R17\n"
                    "    MOV   R22, R16\n"
                    "    RET\n\n");
}