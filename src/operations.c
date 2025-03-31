//
// Created by Rafael Venetikides on 24/03/25.
//

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "../include/stack_operations.h"
#include "../include/operations.h"

#include "../include/serial_operations.h"

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
                    op_multiply(outAsm);
                break;
                case '/':
                    op_real_divide(outAsm);
                break;
                case '|':
                    op_integer_divide(outAsm);
                break;
                case '%':
                    op_modulo(outAsm);
                break;
                case '^':
                    op_power(outAsm);
                break;
                default:
                    printf("Unknown operator '%c'\n", op);
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
    serial_out(outAsm);
}

/**
 *  @brief Adição de dois números de ponto flutuante de meia precisão
 *
 *  @param outAsm Dados de saída do arquivo
 */
void op_add_16bits(FILE *outAsm) {

    fprintf(outAsm, "op_add_16bits:\n\n");


    // R16 = low byte op2
    // R17 = high byte op2

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

    fprintf(outAsm, "; Sinais diferentes - subtração\n"
                    "    ; Determinar qual valor é maior\n"
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
                    "    MOV R27, R31        ; sinal do resultado = sinal de B\n"
                    "    ; Troca R25:R24 ? R23:R22\n"
                    "    MOV R22, R24\n"
                    "    MOV R23, R25\n"
                    "    RJMP normalize\n\n");

    fprintf(outAsm, "same_signs:\n"
                    "    ; Sinais iguais - adição\n"
                    "    ADD R22, R24        ; soma bytes baixos\n"
                    "    ADC R23, R25        ; soma bytes altos com carry\n"
                    "    MOV R27, R30        ; sinal do resultado = sinal de A (ou B, são iguais)\n\n");

    fprintf(outAsm, "normalize:\n"
                    "    ; Verificar se houve overflow na soma e ajustar\n"
                    "    SBRC R23, 3         ; verifica se bit 3 está ligado (overflow)\n"
                    "    RJMP adjust_overflow\n"
                    "\n"
                    "    ; Verificar subflow (mantissa zerada)\n"
                    "    MOV R28, R23\n"
                    "    OR R28, R22         ; R28 = R23 | R22\n"
                    "    BREQ result_zero    ; se resultado for zero\n\n");

    fprintf(outAsm, "    ; Normalizar o resultado (deslocar à esquerda até bit implícito estar na posição correta)\n"
                    "normalize_loop:\n"
                    "    SBRC R23, 2         ; verifica se bit implícito está na posição\n"
                    "    RJMP end_normalize  ; se sim, fim da normalização\n"
                    "    \n"
                    "    LSL R22             ; desloca mantissa à esquerda\n"
                    "    ROL R23\n"
                    "    DEC R26             ; decrementa expoente\n"
                    "    \n"
                    "    ; Verificar subflow de expoente\n"
                    "    CPI R26, 0\n"
                    "    BRNE normalize_loop\n"
                    "    \n"
                    "    ; Resultado denormalizado\n"
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
                    "    LDI R26, 0          ; expoente zero\n"
                    "    LDI R27, 0          ; sinal positivo\n\n");

    fprintf(outAsm, "end_normalize:\n"
                    "    ; Montar resultado final em formato half-precision\n"
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
                    "    ORI R23, 0x80       ; seta bit de sinal se negativo\n\n");

    fprintf(outAsm, "no_sign:\n"
            "   RET\n\n");
}

/**
 * @brief Alinhamento dos expoentes para fazer o cálculo de adição
 *
 * @param outAsm Dados de saída do arquivo
 */
void align_exp(FILE *outAsm) {
    fprintf(outAsm, "; Extrair expoente de R19 (operando 1)\n"
                    "    MOV R20, R19        ; copia high byte \n"
                    "    ANDI R20, 0x7C ; limpa o bit de sinal, deixa os bits 14-10 alinhados\n"
                    "    LSR R20             ; >>1 para alinhar com bits 0–4\n"
                    "    LSR R20             ; Agora R20 tem expoente1\n\n");

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
                    "    ORI R23, 0x04       ; adiciona bit implícito (bit 10 da mantissa)\n\n");

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
                    "    ORI R25, 0x04       ; adiciona bit implícito (bit 10 da mantissa)\n\n");

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

void op_sub_16bits(FILE *outAsm) {

    fprintf(outAsm, "op_sub_16bits:\n\n");

    fprintf(outAsm, "   ORI R19, 0x80\n\n");

    fprintf(outAsm, "   CALL op_add_16bits\n\n");

    fprintf(outAsm, "   RET\n\n");
}

void op_multiply(FILE *outAsm) {

}

void op_real_divide(FILE *outAsm) {

}

void op_integer_divide(FILE *outAsm) {

}

void op_modulo(FILE *outAsm) {

}

void op_power(FILE *outAsm) {

}