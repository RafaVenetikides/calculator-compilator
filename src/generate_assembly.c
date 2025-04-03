//
// Created by Rafael Venetikides on 24/03/25.
//

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "../include/utils/stack_operations.h"
#include "../include/generate_assembly.h"

#include "../include/utils/serial_operations.h"

/**
 * @brief Generates assembly code from a postfix expression
 *
 * @param postfix pointer to the postfix expression
 * @param outAsm output file
 */
void generateAssemblyFromPostfix(const char *postfix, FILE *outAsm) {

    const char *p = postfix;

    while (*p) {
        while (isspace((unsigned char)*p)) {
            p++;
        }
        if (!*p) break;

        if (strchr("+-*/|%^", *p)) {
            char op = *p;
            p++;

            switch (op) {
                case '+':
                    gen_pop_16bit(outAsm);

                    fprintf(outAsm, ";  Moving values from op 1\n"
                                    "    MOV R18, R16         ; copy low byte op1\n"
                                    "    MOV R19, R17         ; copy high byte op1\n");

                    gen_pop_16bit(outAsm);

                    fprintf(outAsm, "   CALL op_add_16bits\n\n");

                    fprintf(outAsm,"   PUSH R22\n"
                                    "   PUSH R23\n\n");
                break;
                case '-':
                    gen_pop_16bit(outAsm);

                    fprintf(outAsm, ";  Moving values from op 1\n"
                                    "    MOV R18, R16         ; copy low byte op1\n"
                                    "    MOV R19, R17         ; copy high byte op1\n\n");

                    gen_pop_16bit(outAsm);

                    fprintf(outAsm, "   CALL op_sub_16bits\n\n");

                    fprintf(outAsm,"   PUSH R22\n"
                                   "   PUSH R23\n\n");
                break;
                case '*':
                    gen_pop_16bit(outAsm);

                    fprintf(outAsm, ";  Moving values from op 1\n"
                                "    MOV R18, R16         ; copy low byte op1\n"
                                "    MOV R19, R17         ; copy high byte op1\n\n");

                    gen_pop_16bit(outAsm);

                    fprintf(outAsm, "   CALL op_mult_16bits\n\n");

                    fprintf(outAsm,"   PUSH R22\n"
                                   "   PUSH R23\n\n");
                break;
                case '|':

                    fprintf(outAsm, "    POP R17\n"
                                    "    POP R16\n\n"
                                    "    ; Pop B\n"
                                    "    POP R19\n"
                                    "    POP R18\n");

                    fprintf(outAsm, "   CALL op_div_16bits\n\n");

                    fprintf(outAsm,"   PUSH R22\n"
                                   "   PUSH R23\n\n");
                break;
                case '/':
                    fprintf(outAsm, "    POP R17\n"
                                    "    POP R16\n\n"
                                    "    ; Pop B\n"
                                    "    POP R19\n"
                                    "    POP R18\n\n");

                    fprintf(outAsm, "   CALL op_div_int_16bits\n\n");

                    fprintf(outAsm,"   PUSH R22\n"
                                   "   PUSH R23\n\n");
                break;
                case '%':
                    fprintf(outAsm, "    POP R17\n"
                                    "    POP R16\n\n"
                                    "    ; Pop B \n"
                                    "    POP R19\n"
                                    "    POP R18\n\n");

                    fprintf(outAsm, "   CALL op_rem_16bits\n\n");

                    fprintf(outAsm,"   PUSH R22\n"
                                   "   PUSH R23\n\n");
                break;
                case '^':
                    fprintf(outAsm, "    POP R17\n"
                                    "    POP R16\n\n");

                    fprintf(outAsm, ";  Moving values from op 1\n"
                                    "    MOV R19, R17\n"
                                    "    MOV R18, R16\n\n");

                    fprintf(outAsm, "    POP R17\n"
                                    "    POP R16\n\n");

                    fprintf(outAsm, "   CALL op_pow_16bits\n\n");

                    fprintf(outAsm,"   PUSH R22\n"
                                   "   PUSH R23\n\n");
                break;
                default:
                    printf("; Unknown operator '%c'\n", op);
                    fprintf(outAsm, "; Unkown operator");
                break;
            }
        } else {
            // É um número, então vamos ler até espaço ou operador
            char temp[64];
            int i = 0;

            // Copia os caracteres do número para temp[]
            while (*p
                && !isspace((unsigned char)*p)
                && !strchr("+-*/|%^", *p))
            {
                temp[i++] = *p;
                p++;
            }
            temp[i] = '\0'; // fim de string

            // Convertemos para int (ou outra função se precisar half-precision etc.)
            float valFloat = strtof(temp, NULL);

            // Agora geramos as instruções de push de 16 bits
            gen_push_16bit(valFloat, outAsm);
        }
    }
    fprintf(outAsm, "   POP R23\n"
                    "   POP R22\n\n");
    fprintf(outAsm, "   CALL print\n\n");
}

void write_functions(FILE *outAsm) {
    fprintf(outAsm, "RJMP program_start\n\n");
    op_add_16bits(outAsm);
    op_sub_16bits(outAsm);
    op_mult_16bits(outAsm);
    op_div_16bits(outAsm);
    op_div_int_16bits(outAsm);
    op_rem_16bits(outAsm);
    op_power(outAsm);
    serial_out(outAsm);
}

/**
 * @brief Alinhamento dos expoentes para fazer o cálculo de adição
 *
 * @param outAsm Dados de saída do arquivo
 */
void align_exp(FILE *outAsm) {
    fprintf(outAsm, "; Extrair expoente de R19 (operando 1)\n"
                    "    MOV R20, R19\n"
                    "    ANDI R20, 0x7C\n"
                    "    LSR R20\n"
                    "    LSR R20\n\n");

    fprintf(outAsm, "; Extrair expoente de R17 (operando 2)\n"
                    "    MOV R21, R17\n"
                    "    ANDI R21, 0x7C\n"
                    "    LSR R21\n"
                    "    LSR R21             ; Agora R21 tem expoente2\n\n");

    fprintf(outAsm, "; Extrair mantissas (10 bits cada, incluindo bit implícito)\n"
                    "    ; Mantissa de A (R19:R18)\n"
                    "    MOV R22, R18        ; byte baixo\n"
                    "    MOV R23, R19        ; byte alto\n"
                    "    ANDI R23, 0x03      ; isola bits 1-0 (bits 9-8 da mantissa)\n\n");

    fprintf(outAsm, "; Adiciona bit implícito se não for denormalizado\n"
                    "    CPI R20, 0          ; verifica se expoente é zero\n"
                    "    BREQ no_implicit_a  ; se zero, não adiciona bit implícito\n"
                    "    ORI R23, 0x04\n\n");

    fprintf(outAsm, "no_implicit_a:\n"
                    "\n"
                    "    ; Mantissa de B (R17:R16)\n"
                    "    MOV R24, R16        ; byte baixo\n"
                    "    MOV R25, R17        ; byte alto\n"
                    "    ANDI R25, 0x03      ; isola bits 1-0 (bits 9-8 da mantissa)\n"
                    "    \n"
                    "    ; Adiciona bit implícito se não for denormalizado\n"
                    "    CPI R21, 0          ; verifica se expoente é zero\n"
                    "    BREQ no_implicit_b  ; se zero, não adiciona bit implícito\n"
                    "    ORI R25, 0x04\n\n");

    fprintf(outAsm, "no_implicit_b:\n"
                    "\n"
                    "; Determinar o expoente maior e alinhar mantissas\n"
                    "    CP R20, R21\n"
                    "    BRGE exp_a_ge_b     ; se expoente A >= B, salta\n"
                    "\n"
                    "; Caso B > A: alinhando mantissa A\n"
                    "    MOV R26, R21        ; salva expoente maior (B)\n"
                    "    SUB R21, R20        ; diferença entre expoentes\n\n");

    fprintf(outAsm, "shift_a:\n"
                    "    CPI R21, 0          ; verifica se ainda há deslocamentos a fazer\n"
                    "    BREQ end_shift_a\n"
                    "    LSR R23             ; desloca mantissa A para direita\n"
                    "    ROR R22\n"
                    "    DEC R21\n"
                    "    RJMP shift_a\n"
                    "end_shift_a:\n"
                    "    RJMP aligned\n\n");

    fprintf(outAsm, "exp_a_ge_b:\n"
                    "; Caso A >= B: alinhando mantissa B\n"
                    "    MOV R26, R20        ; salva expoente maior (A)\n"
                    "    SUB R20, R21        ; diferença entre expoentes\n"
                    "\n"
                    "shift_b:\n"
                    "    CPI R20, 0          ; verifica se ainda há deslocamentos a fazer\n"
                    "    BREQ end_shift_b\n"
                    "    LSR R25             ; desloca mantissa B para direita\n"
                    "    ROR R24\n"
                    "    DEC R20\n"
                    "    RJMP shift_b\n"
                    "end_shift_b:\n\n"
                    "aligned:\n");
}