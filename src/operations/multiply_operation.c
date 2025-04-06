//
// Created by Rafael Venetikides on 03/04/25.
//

#include "../../include/operations/multiply_operation.h"


void op_mult_16bits(FILE *outAsm) {
    fprintf(outAsm, "op_mult_16bits:\n"
                    "   PUSH R21\n"
                    "   PUSH R20\n"
                    "   PUSH R19\n"
                    "   PUSH R18\n"
                    "   PUSH R17\n"
                    "   PUSH R16\n"
                    "   ; --- Extrair sinais ---\n"
                    "   MOV R30, R19\n"
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
                    "   RJMP mult_return\n\n"
                    "exp_overflow:\n"
                    "   CLR R22\n"
                    "   LDI R23, 0x7C\n"
                    "   OR R23, R30\n"
                    "   RJMP mult_return\n\n"
                    "exp_underflow:\n"
                    "   CLR R22\n"
                    "   CLR R23\n"
                    "   OR R23, R30\n"
                    "\n"
                    "mult_return:\n"
                    "   POP R16\n"
                    "   POP R17\n"
                    "   POP R18\n"
                    "   POP R19\n"
                    "   POP R20\n"
                    "   POP R21\n"
                    "   RET\n\n");
}
