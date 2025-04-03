//
// Created by Rafael Venetikides on 03/04/25.
//

#include "../../include/operations/add_operation.h"

/**
 *  @brief Adição de dois números de ponto flutuante de meia precisão
 *
 *  @param outAsm Dados de saída do arquivo
 */
void op_add_16bits(FILE *outAsm) {

    fprintf(outAsm, "op_add_16bits:\n\n");
    fprintf(outAsm, "   PUSH R19\n"
                    "   PUSH R18\n");

    fprintf(outAsm, "; Op1 -> R19:R18\n");
    fprintf(outAsm, "; Op2 -> R17:R16\n\n");

    fprintf(outAsm, "    SBRC R19, 7\n"
                    "    LDI R30, 1    ; sinal_A = 1\n"
                    "    SBRS R19, 7\n"
                    "    LDI R30, 0    ; sinal_A = 0\n\n");

    fprintf(outAsm, "    SBRC R17, 7\n"
                    "    LDI R31, 1    ; sinal_B = 1\n"
                    "    SBRS R17, 7\n"
                    "    LDI R31, 0    ; sinal_B = 0\n\n");

    align_exp(outAsm);

    fprintf(outAsm, "; Agora R30 = sinal_A, R31 = sinal_B\n"
                    "    ; Compare:\n"
                    "    CP R30, R31\n"
                    "    BREQ same_signs");

    fprintf(outAsm,"    ; Determinar qual valor é maior\n"
                    "    CP R23, R25         ; compara byte alto das mantissas\n"
                    "    BRNE high_diff\n"
                    "    CP R22, R24         ; se bytes altos iguais, compara bytes baixos\n\n");

    fprintf(outAsm, "high_diff:\n"
                    "    BRLO b_greater      ; se A < B\n"
                    "\n"
                    "    ; A >= B: R23:R22 - R25:R24\n"
                    "    SUB R22, R24\n"
                    "    SBC R23, R25\n"
                    "    MOV R27, R30        ; sinal do resultado = sinal de A\n"
                    "    RJMP normalize\n\n");

    fprintf(outAsm, "b_greater:\n"
                    "    ; B > A: R25:R24 - R23:R22\n"
                    "    SUB R24, R22\n"
                    "    SBC R25, R23\n"
                    "    MOV R27, R31        ; sinal do resultado = sinal de B\n\n"
                    "    MOV R22, R24\n"
                    "    MOV R23, R25\n"
                    "    RJMP normalize\n\n");

    fprintf(outAsm, "same_signs:\n"
                    "    ; Sinais iguais - adição\n"
                    "    ADD R22, R24        \n"
                    "    ADC R23, R25        \n"
                    "    MOV R27, R30        \n\n");

    fprintf(outAsm, "normalize:\n"
                    "    ; Verificar se houve overflow na soma e ajustar\n"
                    "    SBRC R23, 3\n"
                    "    RJMP adjust_overflow\n"
                    "\n"
                    "    ; Verificar subflow (mantissa zerada)\n"
                    "    MOV R28, R23\n"
                    "    OR R28, R22         ; R28 = R23 | R22\n"
                    "    BREQ result_zero    ; se resultado for zero\n\n");

    fprintf(outAsm, "\n"
                    "normalize_loop:\n"
                    "    SBRC R23, 2\n"
                    "    RJMP end_normalize\n"
                    "    \n"
                    "    LSL R22             ; desloca mantissa à esquerda\n"
                    "    ROL R23\n"
                    "    DEC R26             ; decrementa expoente\n"
                    "    \n"
                    "    ; Verificar subflow de expoente\n"
                    "    CPI R26, 0\n"
                    "    BRNE normalize_loop\n"
                    "    \n"
                    "    RJMP end_normalize\n\n");

    fprintf(outAsm, "adjust_overflow:\n"
                    "    LSR R23             ; desloca à direita para ajustar overflow\n"
                    "    ROR R22\n"
                    "    INC R26             ; incrementa expoente\n"
                    "    \n"
                    "    ; Verificar overflow de expoente (máx 31 = 0x1F)\n"
                    "    CPI R26, 0x1F\n"
                    "    BRNE end_normalize\n"
                    "    \n"
                    "    ; Overflow - definir resultado como infinito\n"
                    "    LDI R23, 0x00\n"
                    "    LDI R22, 0x00\n"
                    "    LDI R26, 0x1F       ; expoente máximo\n"
                    "    RJMP end_normalize\n\n");

    fprintf(outAsm, "result_zero:\n"
                    "    LDI R26, 0\n"
                    "    LDI R27, 0\n\n");

    fprintf(outAsm, "end_normalize:\n"
                    "    ANDI R23, 0x03      ; manter apenas bits 9-8 da mantissa\n"
                    "    \n"
                    "    ; Adicionar expoente\n"
                    "    LSL R26             ; desloca expoente para posição correta\n"
                    "    LSL R26\n"
                    "    ANDI R26, 0x7C      ; garante que apenas bits de expoente estejam setados\n"
                    "    OR R23, R26         ; adiciona expoente ao byte alto\n"
                    "    \n"
                    "    ; Adicionar bit de sinal\n"
                    "    CPI R27, 1\n"
                    "    BRNE no_sign\n"
                    "    ORI R23, 0x80\n\n");

    fprintf(outAsm, "no_sign:\n"
                    "   POP R18\n"
                    "   POP R19\n"
                    "   RET\n\n");
}

