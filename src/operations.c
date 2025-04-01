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

                    fprintf(outAsm, "    POP R17         ; R17=0x40 (A high)\n"
                                    "    POP R16         ; R16=0x00 (A low)   => Divisor = R17:R16 = 0x4000 (2.0)\n\n"
                                    "    ; Pop B (0x4800) diretamente para os registradores do Dividendo (R19:R18)\n"
                                    "    POP R19         ; R19=0x48 (B high)\n"
                                    "    POP R18         ; R18=0x00 (B low)   => Dividendo = R19:R18 = 0x4800 (8.0)\n\n");

                    fprintf(outAsm, "   CALL op_div_16bits\n\n");

                    fprintf(outAsm,"   PUSH R22\n"
                                   "   PUSH R23\n\n");
                break;
                case '/':
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
    op_mult_16bits(outAsm);
    op_div_16bits(outAsm);
    serial_out(outAsm);
}

/**
 *  @brief Adição de dois números de ponto flutuante de meia precisão
 *
 *  @param outAsm Dados de saída do arquivo
 */
void op_add_16bits(FILE *outAsm) {

    fprintf(outAsm, "op_add_16bits:\n\n");
    fprintf(outAsm, "   PUSH R19\n"
                    "   PUSH R18\n");

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
            "   POP R18\n"
            "   POP R19\n"
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

void op_mult_16bits(FILE *outAsm) {
    fprintf(outAsm, "op_mult_16bits:\n"
                    "   ; --- Extrair sinais ---\n"
                    "         MOV R30, R19\n"
                    "   ANDI R30, 0x80\n"
                    "   MOV R31, R17\n"
                    "   ANDI R31, 0x80\n"
                    "   EOR R30, R31       ; Sinal do resultado\n\n"

                    "; --- Verificar zero ---\n"
                    "; Se expoente e mantissa forem zero, é zero\n"
                    "   MOV R20, R19\n"
                    "   ANDI R20, 0x7C\n"
                    "   TST R20\n"
                    "   BRNE check_op2_zero\n"
                    "   MOV R24, R18\n"
                    "   MOV R25, R19\n"
                    "   ANDI R25, 0x03\n"
                    "   OR R24, R25\n"
                    "   BRNE check_op2_zero\n"
                    "   ; Op1 é zero\n"
                    "   CLR R22\n"
                    "   CLR R23\n"
                    "   OR R23, R30\n"
                    "   RET\n\n"

                    "check_op2_zero:\n"
                    "   MOV R21, R17\n"
                    "   ANDI R21, 0x7C\n"
                    "   TST R21\n"
                    "   BRNE extract_fields\n"
                    "   MOV R24, R16\n"
                    "   MOV R25, R17\n"
                    "   ANDI R25, 0x03\n"
                    "   OR R24, R25\n"
                    "   BRNE extract_fields\n"
                    "   ; Op2 é zero\n"
                    "   CLR R22\n"
                    "   CLR R23\n"
                    "   OR R23, R30\n"
                    "   RET\n"

                    "extract_fields:\n"
                    "; --- Extrair mantissas e expoentes ---\n"
                    "        ; Operando 1\n"
                    "   MOV R24, R18\n"
                    "   MOV R25, R19\n"
                    "   ANDI R25, 0x03\n"
                    "   MOV R20, R19\n"
                    "   ANDI R20, 0x7C\n"
                    "   LSR R20\n"
                    "   LSR R20\n"
                    "   CPI R20, 0\n"
                    "   BRNE op1_norm\n"
                    "   ORI R25, 0x00\n"
                    "   LDI R20, 1\n"
                    "   RJMP op1_ready\n"
                    "op1_norm:\n"
                    "   ORI R25, 0x04\n"
                    "op1_ready:\n"

                    "   ; Operando 2\n"
                    "   MOV R26, R16\n"
                    "   MOV R27, R17\n"
                    "   ANDI R27, 0x03\n"
                    "   MOV R21, R17\n"
                    "   ANDI R21, 0x7C\n"
                    "   LSR R21\n"
                    "   LSR R21\n"
                    "   CPI R21, 0\n"
                    "   BRNE op2_norm\n"
                    "   ORI R27, 0x00\n"
                    "   LDI R21, 1\n"
                    "   RJMP op2_ready\n"
                    "op2_norm:\n"
                    "   ORI R27, 0x04\n"
                    "op2_ready:\n"

                    "; --- Calcular expoente ---\n"
                    "   ADD R20, R21\n"
                    "   SUBI R20, 15\n\n"

                    "; --- Multiplicar mantissas: (R25:R24) * (R27:R26) ---\n"
                    "; Resultado 32 bits em R0-R3\n"
                    "   CLR R0\n"
                    "   CLR R1\n"
                    "   CLR R2\n"
                    "   CLR R3\n\n"

                    "   MUL R24, R26\n"
                    "   MOV R0, r0\n"
                    "   MOV R1, r1\n\n"

                    "   MUL R24, R27\n"
                    "   ADD R1, r0\n"
                    "   ADC R2, r1\n"
                    "   CLR r18\n"
                    "   ADC R3, r18\n\n"

                    "   MUL R25, R26\n"
                    "   ADD R1, r0\n"
                    "   ADC R2, r1\n"
                    "   CLR r18\n"
                    "   ADC R3, r18\n\n"

                    "   MUL R25, R27\n"
                    "   ADD R2, r0\n"
                    "   ADC R3, r1\n\n"

                    "; Produto em R3:R2:R1:R0 (22 bits úteis)\n"
                    "; Verifica overflow e ajusta expoente\n"
                    "   SBRC R2, 5\n"
                    "   RJMP shift_prod\n"
                    "   RJMP format_mantissa\n"

                    "shift_prod:\n"
                    "   LSR R3\n"
                    "   ROR R2\n"
                    "   ROR R1\n"
                    "   ROR R0\n"
                    "   INC R20\n\n"

                    "format_mantissa:\n"
                    "; Verificar overflow/underflow de expoente\n"
                    "   CPI R20, 31\n"
                    "   BRGE exp_overflow\n"
                    "   CPI R20, 1\n"
                    "   BRLT exp_underflow\n\n"

                    "; Extrair 10 bits após o bit implícito (bit 20)\n"
                    "; Byte alto: bits 19-18 (bits 3-2 de R2)\n"
                    "; Byte baixo: bits 17-10 (bits 1-0 de R2 + 7-2 de R1)\n\n"

                    "   MOV R23, R30\n"
                    "   MOV R24, R20\n"
                    "   LSL R24\n"
                    "   LSL R24\n"
                    "   ANDI R24, 0x7C\n"
                    "   OR R23, R24\n\n"

                    "   MOV R24, R2\n"
                    "   ANDI R24, 0x0C\n"
                    "   LSR R24\n"
                    "   LSR R24\n"
                    "   OR R23, R24\n\n"

                    "   MOV R22, R2\n"
                    "   ANDI R22, 0x03\n"
                    "   LSL R22\n"
                    "   LSL R22\n"
                    "   LSL R22\n"
                    "   LSL R22\n"
                    "   LSL R22\n"
                    "   LSL R22\n"
                    "   MOV R24, R1\n"
                    "   ANDI R24, 0xFC\n"
                    "   LSR R24\n"
                    "   LSR R24\n"
                    "   OR R22, R24\n"
                    "   RET\n\n"

                    "exp_overflow:\n"
                    "   CLR R22\n"
                    "   LDI R23, 0x7C\n"
                    "   OR R23, R30\n"
                    "   RET\n\n"

                    "exp_underflow:\n"
                    "   CLR R22\n"
                    "   CLR R23\n"
                    "   OR R23, R30\n"
                    "   RET\n\n");
}

void op_div_16bits(FILE *outAsm) {
    fprintf(outAsm, "op_div_16bits:\n"
                    "    ; --- Sign Handling ---\n"
                    "    MOV R30, R19\n"
                    "    ANDI R30, 0x80         ; Sign A\n"
                    "    MOV R31, R17\n"
                    "    ANDI R31, 0x80         ; Sign B\n"
                    "    EOR R30, R31           ; R30 = Result Sign (0x80 or 0x00)\n"
                    "\n"
                    "    ; --- Special Case Handling ---\n"
                    "    ; Check A for NaN or Infinity\n"
                    "    MOV R20, R19           ; R20 = A_high\n"
                    "    ANDI R20, 0x7C         ; Isolate A exponent bits\n"
                    "    CPI R20, 0x7C          ; Exponent = 0x1F?\n"
                    "    BRNE check_b_nan_inf   ; If not, check B\n"
                    "    ; A has exponent 0x1F (Inf or NaN)\n"
                    "    MOV R21, R19           ; Check A mantissa\n"
                    "    ANDI R21, 0x03\n"
                    "    OR R21, R18            ; R21 = A_mantissa_low | A_mantissa_high_bits\n"
                    "    BRNE result_nan_quietA ; If A mantissa != 0, A is NaN -> Result is NaN (quiet A)\n"
                    "\n"
                    "    ; A is Infinity\n"
                    "    MOV R22, R17           ; R22 = B_high\n"
                    "    ANDI R22, 0x7C         ; Isolate B exponent bits\n"
                    "    CPI R22, 0x7C          ; Is B exponent 0x1F?\n"
                    "    BRNE pack_infinity        ; If B is not Inf/NaN, result is Inf (A/finite = Inf)\n"
                    "    ; B also has exponent 0x1F (Inf or NaN)\n"
                    "    MOV R23, R17           ; Check B mantissa\n"
                    "    ANDI R23, 0x03\n"
                    "    OR R23, R16            ; R23 = B_mantissa\n"
                    "    BRNE result_nan_quietB ; If B mantissa != 0, B is NaN -> Result is NaN (quiet B)\n"
                    "\n"
                    "    ; Both A and B are Infinity -> Indeterminate -> NaN\n"
                    "    RJMP result_nan        ; Quiet NaN as default for Inf/Inf\n"
                    "check_b_nan_inf:\n"
                    "    ; Check B for NaN or Infinity\n"
                    "    MOV R22, R17           ; R22 = B_high\n"
                    "    ANDI R22, 0x7C         ; Isolate B exponent bits\n"
                    "    CPI R22, 0x7C          ; Exponent = 0x1F?\n"
                    "    BRNE check_a_zero      ; If not, check if A is zero\n"
                    "\n"
                    "    ; B has exponent 0x1F (Inf or NaN)\n"
                    "    MOV R23, R17           ; Check B mantissa\n"
                    "    ANDI R23, 0x03\n"
                    "    OR R23, R16            ; R23 = B_mantissa\n"
                    "    BRNE result_nan_quietB ; If B mantissa != 0, B is NaN -> Result is NaN (quiet B)\n"
                    "    ; B is Infinity, A is Finite (checked earlier) -> Result is Zero\n"
                    "    CLR R22                ; Result Low = 0\n"
                    "    MOV R23, R30           ; Result High = Sign\n"
                    "    RET\n"
                    "\n"
                    "check_a_zero:\n"
                    "    ; Check if A is Zero (Exponent and Mantissa are 0)\n"
                    "    MOV R20, R19           ; R20 = A_high\n"
                    "    ANDI R20, 0x7F         ; Ignore sign bit\n"
                    "    OR R20, R18            ; Combine High (exp+mant) and Low (mant)\n"
                    "    BRNE check_b_zero      ; If A != 0, check if B is zero\n"
                    "    ; A is Zero\n"
                    "    ; Check if B is also Zero (handled next) to avoid 0/0 -> NaN here\n"
                    "    MOV R22, R17           ; Check B for zero\n"
                    "    ANDI R22, 0x7F\n"
                    "    OR R22, R16\n"
                    "    BRNE div_a_is_zero     ; If B != 0, result is Zero\n"
                    "\n"
                    "    ; A is Zero AND B is Zero -> 0/0 -> NaN\n"
                    "    RJMP result_nan\n"
                    "div_a_is_zero:\n"
                    "    ; A is Zero, B is non-Zero -> Result is Zero\n"
                    "    CLR R22                ; Result Low = 0\n"
                    "    MOV R23, R30           ; Result High = Sign\n"
                    "    RET\n"
                    "\n"
                    "check_b_zero:\n"
                    "    ; Check if B is Zero (Exponent and Mantissa are 0)\n"
                    "    MOV R22, R17           ; R22 = B_high\n"
                    "    ANDI R22, 0x7F         ; Ignore sign bit\n"
                    "    OR R22, R16            ; Combine High (exp+mant) and Low (mant)\n"
                    "    BRNE continue_division ; If B != 0, proceed to division\n"
                    "    ; B is Zero, A is non-Zero -> Division by Zero -> Infinity\n"
                    "    LDI R22, 0x00          ; Mantissa = 0\n"
                    "    LDI R23, 0x7C          ; Exponent = 0x1F\n"
                    "    OR R23, R30            ; Add sign\n"
                    "    RET\n"
                    "\n"
                    "; --- Result Handling for Special Cases ---\n"
                    "pack_infinity:\n"
                    "    ; Called when result is +/- Infinity\n"
                    "    LDI R22, 0x00          ; Mantissa = 0\n"
                    "    LDI R23, 0x7C          ; Exponent = 0x1F\n"
                    "    OR R23, R30            ; Add sign\n"
                    "    RET\n"
                    "result_nan_quietA:\n"
                    "    ; A is NaN, make result quiet NaN based on A's mantissa if possible\n"
                    "    MOV R22, R18           ; Low part of A's mantissa\n"
                    "    MOV R23, R19           ; High part of A's mantissa + exponent\n"
                    "    ORI R23, 0x7C          ; Ensure exponent is 0x1F (NaN)\n"
                    "    ORI R23, 0x02          ; Ensure mantissa MSb is 1 (Quiet NaN)\n"
                    "    RET\n"
                    "\n"
                    "result_nan_quietB:\n"
                    "    ; B is NaN, make result quiet NaN based on B's mantissa if possible\n"
                    "    MOV R22, R16           ; Low part of B's mantissa\n"
                    "    MOV R23, R17           ; High part of B's mantissa + exponent\n"
                    "    ORI R23, 0x7C          ; Ensure exponent is 0x1F (NaN)\n"
                    "    ORI R23, 0x02          ; Ensure mantissa MSb is 1 (Quiet NaN)\n"
                    "    RET\n"
                    "result_nan:\n"
                    "    ; Generic Quiet NaN for cases like Inf/Inf, 0/0\n"
                    "    LDI R22, 0x01          ; Non-zero mantissa\n"
                    "    LDI R23, 0x7E          ; Exp=0x1F, MantMSB=1 -> 0b0111 1110\n"
                    "    ; Note: Sign bit is typically ignored for NaN, but can be preserved if needed\n"
                    "    ; OR R23, R30          ; Optional: preserve sign\n"
                    "    RET\n"
                    "\n"
                    "; --- Start Normal Division ---\n"
                    "continue_division:\n"
                    "    ; --- Extract Exponents ---\n"
                    "    ; R20 = Exponent A (biased)\n"
                    "    MOV R20, R19\n"
                    "    ANDI R20, 0x7C         ; Isolate exponent bits\n"
                    "    LSR R20\n"
                    "    LSR R20                ; R20 = biased exponent A (0-31)\n"
                    "    ; R21 = Exponent B (biased)\n"
                    "    MOV R21, R17\n"
                    "    ANDI R21, 0x7C         ; Isolate exponent bits\n"
                    "    LSR R21\n"
                    "    LSR R21                ; R21 = biased exponent B (0-31)\n"
                    "\n"
                    "    ; --- Extract Mantissas and add Implicit Bit ---\n"
                    "    ; Mantissa A in R25:R24 (11 bits including implicit 1)\n"
                    "    MOV R24, R18           ; Low byte A\n"
                    "    MOV R25, R19           ; High byte A\n"
                    "    ANDI R25, 0x03         ; Clear exponent/sign, keep mantissa bits\n"
                    "    CPI R20, 0             ; Check if A is denormal (exponent == 0)\n"
                    "    BRNE a_normal\n"
                    "    ; A is denormal: exponent = 0. Effective exponent is 1. No implicit bit.\n"
                    "    LDI R20, 1             ; Use exponent=1 for calculation\n"
                    "    RJMP a_ready\n"
                    "a_normal:\n"
                    "    ; A is normal: Add implicit bit (bit 10) -> 0x0400\n"
                    "    ORI R25, 0x04          ; Set implicit bit in high byte part\n"
                    "a_ready:\n"
                    "    ; R25:R24 = Mantissa A (11 bits, format HHHH HHHH LLLL L000 for implicit bit)\n"
                    "    ; Mantissa B in R27:R26 (11 bits including implicit 1)\n"
                    "    MOV R26, R16           ; Low byte B\n"
                    "    MOV R27, R17           ; High byte B\n"
                    "    ANDI R27, 0x03         ; Clear exponent/sign, keep mantissa bits\n"
                    "    CPI R21, 0             ; Check if B is denormal (exponent == 0)\n"
                    "    BRNE b_normal\n"
                    "    ; B is denormal: exponent = 0. Effective exponent is 1. No implicit bit.\n"
                    "    LDI R21, 1             ; Use exponent=1 for calculation\n"
                    "    RJMP b_ready\n"
                    "b_normal:\n"
                    "    ; B is normal: Add implicit bit\n"
                    "    ORI R27, 0x04          ; Set implicit bit\n"
                    "b_ready:\n"
                    "\n"
                    "    SUB R20, R21           ; R20 = exp_A - exp_B\n"
                    "    SUBI R20, 241          ; Add bias (15) using SUBI R20, -15 (2's complement)\n"
                    "    CLR R2                 ; High byte\n"
                    "    MOV R3, R25            ; Mid byte (from high byte of MantA)\n"
                    "    MOV R4, R24            ; Low byte (from low byte of MantA)\n"
                    "    ; Shift R3:R4 left 4 times (Logical shift)\n"
                    "    LDI R16, 4              ; Shift counter\n"
                    "shift_dividend_prep:\n"
                    "    LSL R4\n"
                    "    ROL R3\n"
                    "    ROL R2                 ; R2 gets the highest bits\n"
                    "    DEC R16\n"
                    "    BRNE shift_dividend_prep\n"
                    "\n"
                    "    LDI R16, 8\n"
                    "shift_dividend_main:\n"
                    "    LSL R4\n"
                    "    ROL R3\n"
                    "    ROL R2\n"
                    "    DEC R16\n"
                    "    BRNE shift_dividend_main\n"
                    "    CLR R1                 ; Quotient high byte\n"
                    "    CLR R0                 ; Quotient low byte\n"
                    "    CLR R6                 ; Temporary carry/flag register\n"
                    "    LDI R16, 16             ; Loop counter for 16 bits of quotient\n"
                    "\n"
                    "div_loop:\n"
                    "    LSL R4\n"
                    "    ROL R3\n"
                    "    ROL R2                 ; Bit shifted out of R2 goes into Carry\n"
                    "    ROL R0\n"
                    "    ROL R1\n"
                    "\n"
                    "    MOV R5, R2\n"
                    "    SUB R5, R27            ; Subtract high bytes\n"
                    "    MOV R6, R3\n"
                    "    SBC R6, R26            ; Subtract low bytes with borrow\n"
                    "    BRCS div_no_subtract   ; If Carry is set (Borrow occurred), subtraction failed\n"
                    "\n"
                    "    ; Subtraction successful: Update remainder and set quotient bit\n"
                    "    MOV R2, R5             ; Update high byte of remainder\n"
                    "    MOV R3, R6             ; Update low byte of remainder\n"
                    "    LDI R16, 0x01\n"
                    "    OR R0, R16\n"
                    "    RJMP div_iter_done\n"
                    "div_no_subtract:\n"
                    "    ; Subtraction failed: Remainder unchanged, quotient bit remains 0 (already shifted)\n"
                    "    NOP\n"
                    "\n"
                    "div_iter_done:\n"
                    "    DEC R16\n"
                    "    BRNE div_loop\n"
                    "    TST R1                 ; Check high byte of quotient\n"
                    "    BRMI norm_shift_right  ; If MSB (bit 15) is set - should not happen if A/B < 2^4\n"
                    "    ; Check bit 12 (0x10 in R1)\n"
                    "    SBRS R1, 4             ; Skip if bit 12 (0x1000) is not set\n"
                    "    RJMP norm_shift_right  ; Jump if bit 12 is set (MantA >= MantB)\n"
                    "\n"
                    "norm_find_msb:\n"
                    "    ; If Q is zero (extremely unlikely unless inputs were denormal), handle as zero\n"
                    "    OR R1, R0\n"
                    "    BRNE norm_not_zero\n"
                    "    RJMP pack_zero\n"
                    "norm_not_zero:\n"
                    "    LDI R16, 0              ; R7 will count shifts\n"
                    "norm_shift_left_loop:\n"
                    "    ; Check if bit 11 (0x08 in R1) is set\n"
                    "    SBRS R1, 3             ; Skip if bit 11 is set\n"
                    "    RJMP found_msb_pos     ; Found position, exit loop\n"
                    "\n"
                    "    ; Shift quotient left by 1\n"
                    "    LSL R0\n"
                    "    ROL R1\n"
                    "    INC R16                 ; Increment shift count\n"
                    "    CPI R16, 16             ; Safety break (shouldn't happen)\n"
                    "    BREQ norm_done         ; Error or zero case\n"
                    "    RJMP norm_shift_left_loop ; Continue shifting\n"
                    "found_msb_pos:\n"
                    "    ; Shifted left R7 times. Adjust exponent.\n"
                    "    SUB R20, R16            ; Decrement exponent by shift count\n"
                    "    RJMP norm_done\n"
                    "\n"
                    "norm_shift_right:\n"
                    "    CLR R7                 ; R7 = Sticky bit inicial\n"
                    "    OR R2, R3              ; R2 != 0 ou R3 != 0 ? (Resto da divisão)\n"
                    "    BRNE set_sticky_s1\n"
                    "    TST R0                 ; LSB original de Q antes de qualquer shift\n"
                    "    BRNE set_sticky_s1\n"
                    "    RJMP sticky_done_s1\n"
                    "set_sticky_s1:\n"
                    "    LDI R16, 1             ; Correção: usar LDI+OR para R7\n"
                    "    OR R7, R16             ; sticky_s1 = 1\n"
                    "sticky_done_s1:\n"
                    "\n"
                    "    MOV R16, R0            ; Guarda LSB original em R16 (será Guard_s1)\n"
                    "    ANDI R16, 0x01\n"
                    "    ; R16 = Guard_s1\n"
                    "\n"
                    "    ; Realiza o PRIMEIRO shift right\n"
                    "    LSR R1\n"
                    "    ROR R0                  ; <<--- CORRIJA PARA RR AQUI\n"
                    "    ; Agora R1:R0 = Q >> 1\n"
                    "\n"
                    "    ; Verifica se MAIS um shift é necessário (Bit 11 ainda setado?)\n"
                    "    ; Bit 11 está em R1, bit 3 (0x08)\n"
                    "    SBRS R1, 3             ; Pula se bit 11 NÃO estiver setado\n"
                    "    RJMP shift_right_2     ; Se bit 11 SETADO, precisa de mais um shift\n"
                    "\n"
                    "    ; --- Se UM shift foi suficiente ---\n"
                    "    ; Guard final G = Guard_s1 (em R16)\n"
                    "    ; Sticky final S = Sticky_s1 (em R7)\n"
                    "    MOV R6, R16            ; Copia G para R6 (se for usar TST R6 depois)\n"
                    "    RJMP rounding_check    ; Pula para a lógica de arredondamento\n"
                    "\n"
                    "shift_right_2:\n"
                    "    ; --- Se DOIS shifts foram necessários ---\n"
                    "\n"
                    "    ; Calcula Sticky e Guard para o SEGUNDO shift\n"
                    "    ; Sticky_s2 = Sticky_s1 | Guard_s1\n"
                    "    OR R7, R16             ; R7 agora é Sticky_s2\n"
                    "\n"
                    "    ; Guard_s2 = LSB de (Q >> 1) - está em R0, bit 0\n"
                    "    MOV R16, R0            ; Guarda LSB atual em R16 (será Guard_s2)\n"
                    "    ANDI R16, 0x01\n"
                    "    ; R16 = Guard_s2\n"
                    "\n"
                    "    ; Realiza o SEGUNDO shift right\n"
                    "    LSR R1\n"
                    "    ROR R0                  ; <<--- CORRIJA PARA RR AQUI\n"
                    "    ; Agora R1:R0 = Q >> 2 (Normalizado para 0x04xx)\n"
                    "\n"
                    "    ; Guard final G = Guard_s2 (em R16)\n"
                    "    ; Sticky final S = Sticky_s2 (em R7)\n"
                    "    MOV R6, R16            ; Copia G para R6 (se for usar TST R6 depois)\n"
                    "    ; Continua para a lógica de arredondamento\n"
                    "\n"
                    "rounding_check:            ; Ponto comum após 1 ou 2 shifts\n"
                    "    TST R6                 ; Testa Guard bit final (em R6)\n"
                    "    BREQ norm_done         ; G=0, trunca\n"
                    "    ; G=1...\n"
                    "    TST R7                 ; Testa Sticky bit final (em R7)\n"
                    "    BRNE round_up          ; S=1, arredonda pra cima\n"
                    "    ; G=1, S=0 (Empate)...\n"
                    "    SBRS R0, 0             ; Testa LSB do resultado final R0\n"
                    "    RJMP round_up          ; LSB=1, arredonda pra cima (par)\n"
                    "    ; LSB=0, arredonda pra baixo (mantém)\n"
                    "    RJMP norm_done\n"
                    "\n"
                    "round_up:\n"
                    "    ; Increment the shifted quotient R1:R0\n"
                    "    INC R0        ; Add 1 to R0, carry propagates to R1\n"
                    "    BRNE skip_inc_r1\n"
                    "    INC r1\n"
                    "\n"
                    "skip_inc_r1:\n"
                    "    ; Check if rounding caused mantissa overflow (result became 0x0800 - implicit bit now at pos 12)\n"
                    "    SBRS R1, 3             ; Skip if bit 11 (0x0800) is not set after rounding\n"
                    "    RJMP norm_done         ; No overflow\n"
                    "    INC R20\n"
                    "\n"
                    "norm_done:\n"
                    "    ; --- Exponent Overflow/Underflow Check ---\n"
                    "check_exponent:\n"
                    "    ; Check for Overflow (Exponent > 30)\n"
                    "    CPI R20, 31            ; Compare with max exponent + 1\n"
                    "    BRLO exp_not_overflow\n"
                    "    RJMP pack_infinity\n"
                    "\n"
                    "exp_not_overflow:\n"
                    "    ; Check for Underflow (Exponent <= 0)\n"
                    "    CPI R20, 1             ; Compare with min normal exponent\n"
                    "    BRLO handle_underflow  ; If R20 < 1 (i.e., <= 0), handle as denormal/zero\n"
                    "    ; Exponent is in normal range [1, 30]\n"
                    "    RJMP pack_normal\n"
                    "\n"
                    "handle_underflow:\n"
                    " MOV R16, R20           ; R16 = exponent\n"
                    " NEG R16\n"
                    " INC R16                ; R16 = shift amount\n"
                    " CLR R6                 ; R6 = sticky bit\n"
                    "underflow_shift_loop:\n"
                    " CPI R16, 0\n"
                    " BREQ pack_denormal     ; Done shifting, Guard bit is now in Carry flag\n"
                    "\n"
                    " ; Check bit being shifted out (LSB of R0) for sticky calculation BEFORE shift\n"
                    " SEC                    ; Assume LSB=1 (will set sticky)\n"
                    " SBRC R0, 0             ; Skip if LSB is 0\n"
                    " CLC                    ; LSB was 0, clear carry\n"
                    " OR R6, R6              ; OR sticky with itself (to preserve)\n"
                    " BRCC skip_set_sticky   ; If Carry clear (LSB was 0), skip setting sticky\n"
                    " LDI R17, 1             ; Need another temp reg (or use ORI on R16-R31 if R6 was high)\n"
                    " OR R6, R17             ; Set sticky bit R6=1\n"
                    "skip_set_sticky:\n"
                    "\n"
                    " ; Shift R1:R0 right by 1. Carry OUT holds bit that becomes Guard on last iteration.\n"
                    " LSR R1\n"
                    " ROR R0\n"
                    "\n"
                    " DEC R16\n"
                    " RJMP underflow_shift_loop\n"
                    "\n"
                    "pack_denormal:\n"
                    " ; Guard bit G is now in the Carry flag (C) from the last RR R0\n"
                    " ; Sticky bit S is in R6\n"
                    "\n"
                    " ; Rounding check: Need to round up if C=1 and (R6=1 or LSB(R0)=1)\n"
                    " BRCS denorm_guard_is_1 ; Jump if C=1 (G=1)\n"
                    " ; G=0, round down (truncate)\n"
                    " RJMP pack_denorm_final\n"
                    "denorm_guard_is_1:\n"
                    "     ; G=1\n"
                    "     TST R6                 ; Check Sticky bit S in R6\n"
                    "     BRNE denorm_round_up   ; If S=1, round up\n"
                    "\n"
                    "     ; G=1, S=0 (Tie case)\n"
                    "     SBRS R0, 0             ; Check LSB of shifted result R0\n"
                    "     RJMP denorm_round_up   ; If LSB=1, round up (to even)\n"
                    "\n"
                    "     ; G=1, S=0, LSB=0 -> Round down\n"
                    "     RJMP pack_denorm_final\n"
                    "\n"
                    "denorm_round_up:\n"
                    "     ; Increment R1:R0 manually\n"
                    "     INC R0\n"
                    "     BRNE skip_inc_r1_denorm\n"
                    "     INC R1\n"
                    "skip_inc_r1_denorm:\n"
                    "     ; Check if rounding a denormal caused it to become normal (0x0400)\n"
                    "     LDI R16, 0x00          ; Use R16 for comparison\n"
                    "     CP R0, R16\n"
                    "     BRNE pack_denorm_final\n"
                    "     LDI R16, 0x04\n"
                    "     CP R1, R16\n"
                    "     BRNE pack_denorm_final\n"
                    "     ; Became smallest normal, set exponent to 1\n"
                    "     LDI R20, 1\n"
                    "     RJMP pack_normal\n"
                    "\n"
                    "pack_denorm_final:\n"
                    "     CLR R20\n"
                    "     RJMP pack_final\n"
                    "\n"
                    "pack_zero:             ; <<<===== ADICIONAR LABEL AQUI\n"
                    "    ; Set result to +/- Zero (sign preserved)\n"
                    "    CLR R22\n"
                    "    MOV R23, R30\n"
                    "    RET\n"
                    "\n"
                    "; (Label para empacotar números normais)\n"
                    "pack_normal:           ; <<<===== ADICIONAR LABEL AQUI\n"
                    "    MOV R22, R0\n"
                    "    ; High byte R23\n"
                    "    MOV R23, R1            ; R1 holds bits 15..8\n"
                    "    ANDI R23, 0x03         ; Keep only bits 9 and 8 (Mantissa high bits)\n"
                    "    ; Shift exponent R20 left by 2 to align with bits 6..2 in R23\n"
                    "    LSL R20\n"
                    "    LSL R20\n"
                    "    ANDI R20, 0x7C         ; Ensure exponent fits in 5 bits (mask just in case)\n"
                    "    OR R23, R20            ; Combine exponent\n"
                    "    OR R23, R30            ; Combine sign\n"
                    "    RET\n"
                    "\n"
                    "pack_final:\n"
                    "    MOV R22, R0\n"
                    "\n"
                    "    ; High byte R23\n"
                    "    MOV R23, R1            ; R1 holds bits 15..8\n"
                    "    ANDI R23, 0x03         ; Keep only bits 9 and 8 (Mantissa high bits)\n"
                    "    ; Exponent is 0, so no need to OR R20\n"
                    "    OR R23, R30            ; Combine sign\n"
                    "    RET\n");
}

void op_integer_divide(FILE *outAsm) {

}

void op_modulo(FILE *outAsm) {

}

void op_power(FILE *outAsm) {

}