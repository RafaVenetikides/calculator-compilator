//
// Created by Rafael Venetikides on 27/03/25.
//

#include "../include/serial_operations.h"

void serial_functions(FILE *outAsm) {
    fprintf(outAsm, "program_start:\n\n");

    fprintf(outAsm, "; === Inicialização da USART0 para 9600 baud ===\n"
    "    ; Clock = 16 MHz → UBRR = (16*10^6)/(16*9600) - 1 = 103\n"
    "    LDI R16, 0x67\n"
    "    STS UBRR0L, R16\n"
    "    LDI R16, 0x00\n"
    "    STS UBRR0H, R16\n\n"

    "    ; Habilita transmissor\n"
    "    LDI R16, 0x08\n"
    "    STS UCSR0B, R16\n\n"

    "    ; Modo: assíncrono, 8 bits, sem paridade, 1 stop\n"
    "    LDI R16, 0x06\n"
    "    STS UCSR0C, R16\n\n");
}

void serial_out(FILE *outAsm) {

    fprintf(outAsm, "   POP R23\n"
                    "   POP R22\n\n");


    fprintf(outAsm,"    ; === Envia resultado (R23:R22) em HEX ===\n"
                    "    MOV R16, R23\n"
                    "    RCALL send_byte_hex\n"
                    "\n"
                    "    MOV R16, R22\n"
                    "    RCALL send_byte_hex\n"
                    "\n"
                    "    ; envia quebra de linha\n"
                    "    LDI R16, 0x0A\n"
                    "    RCALL send_byte\n"
                    "\n"
                    "    RJMP end\n\n");

    fprintf(outAsm, "; === Função: envia o byte em R16 via serial ===\n"
                    "send_byte:\n"
                    "    ; Espera UDR vazio\n"
                    "wait_udr:\n"
                    "    LDS R17, UCSR0A\n"
                    "    SBRS R17, UDRE0\n"
                    "    RJMP wait_udr\n"
                    "\n"
                    "    ; Envia byte\n"
                    "    STS UDR0, R16\n"
                    "    RET\n\n");

    fprintf(outAsm, "; === Função: envia R16 como 2 dígitos HEX (ASCII) ===\n"
            "send_byte_hex:\n"
            "    PUSH R16        \n"
            "    PUSH R17\n"
            "    PUSH R18\n"
            "    PUSH R19\n"
            "\n"
            "    ; nibble alto\n"
            "    MOV  R17, R16   ; cópia\n"
            "    SWAP R17\n"
            "    ANDI R17, 0x0F\n"
            "    CPI  R17, 10\n"
            "    BRLT hex_digit1\n"
            "    LDI  R19, 'A'\n"
            "    SUBI R19, 10\n"
            "    ADD  R17, R19\n"
            "    RJMP nibble_high_ready\n\n");

    fprintf(outAsm, "hex_digit1:\n"
                    "    LDI R19, '0'\n"
                    "    ADD R17, R19\n"
                    "\n"
                    "nibble_high_ready:\n"
                    "    MOV R16, R17\n"
                    "    RCALL send_byte\n"
                    "    \n"
                    "    ; nibble baixo\n"
                    "    POP R19\n"
                    "    POP R18\n"
                    "    POP R17\n"
                    "    POP R16\n"
                    "    \n"
                    "    PUSH R16\n"
                    "    PUSH R17\n"
                    "    PUSH R18\n"
                    "    PUSH R19\n"
                    "    \n"
                    "    ANDI R16, 0x0F\n"
                    "    CPI  R16, 10\n"
                    "    BRLT hex_digit2\n"
                    "    LDI  R19, 'A'\n"
                    "    SUBI R19, 10\n"
                    "    ADD  R16, R19\n"
                    "    RJMP nibble_low_ready\n\n");

    fprintf(outAsm, "hex_digit2:\n"
                    "    LDI R19, '0'\n"
                    "    ADD R16, R19\n"
                    "\n"
                    "nibble_low_ready:\n"
                    "    RCALL send_byte\n"
                    "\n"
                    "    ; Restaura a pilha na ordem inversa\n"
                    "    POP R19\n"
                    "    POP R18\n"
                    "    POP R17\n"
                    "    POP R16\n"
                    "    RET\n"
                    "\n"
                    "end:\n"
                    "    RJMP end");
}