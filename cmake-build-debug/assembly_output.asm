#include "m328Pdef.inc"

RJMP program_start

op_add_16bits:

; Op1 -> R19:R18
; Op2 -> R17:R16

    SBRC R19, 7
    LDI R30, 1    ; sinal_A = 1
    SBRS R19, 7
    LDI R30, 0    ; sinal_A = 0

    SBRC R17, 7
    LDI R31, 1    ; sinal_B = 1
    SBRS R17, 7
    LDI R31, 0    ; sinal_B = 0

; Extrair expoente de R19 (operando 1)
    MOV R20, R19        ; copia high byte 
    ANDI R20, 0x7C ; limpa o bit de sinal, deixa os bits 14-10 alinhados
    LSR R20             ; >>1 para alinhar com bits 0–4
    LSR R20             ; Agora R20 tem expoente1

; Extrair expoente de R17 (operando 2)
    MOV R21, R17
    ANDI R21, 0x7C
    LSR R21
    LSR R21             ; Agora R21 tem expoente2

; Extrair mantissas (10 bits cada, incluindo bit implícito)
    ; Mantissa de A (R19:R18)
    MOV R22, R18        ; byte baixo
    MOV R23, R19        ; byte alto
    ANDI R23, 0x03      ; isola bits 1-0 (bits 9-8 da mantissa)

; Adiciona bit implícito se não for denormalizado
    CPI R20, 0          ; verifica se expoente é zero
    BREQ no_implicit_a  ; se zero, não adiciona bit implícito
    ORI R23, 0x04       ; adiciona bit implícito (bit 10 da mantissa)

no_implicit_a:

    ; Mantissa de B (R17:R16)
    MOV R24, R16        ; byte baixo
    MOV R25, R17        ; byte alto
    ANDI R25, 0x03      ; isola bits 1-0 (bits 9-8 da mantissa)
    
    ; Adiciona bit implícito se não for denormalizado
    CPI R21, 0          ; verifica se expoente é zero
    BREQ no_implicit_b  ; se zero, não adiciona bit implícito
    ORI R25, 0x04       ; adiciona bit implícito (bit 10 da mantissa)

no_implicit_b:

; Determinar o expoente maior e alinhar mantissas
    CP R20, R21
    BRGE exp_a_ge_b     ; se expoente A >= B, salta

; Caso B > A: alinhando mantissa A
    MOV R26, R21        ; salva expoente maior (B)
    SUB R21, R20        ; diferença entre expoentes

shift_a:
    CPI R21, 0          ; verifica se ainda há deslocamentos a fazer
    BREQ end_shift_a
    LSR R23             ; desloca mantissa A para direita
    ROR R22
    DEC R21
    RJMP shift_a
end_shift_a:
    RJMP aligned

exp_a_ge_b:
; Caso A >= B: alinhando mantissa B
    MOV R26, R20        ; salva expoente maior (A)
    SUB R20, R21        ; diferença entre expoentes

shift_b:
    CPI R20, 0          ; verifica se ainda há deslocamentos a fazer
    BREQ end_shift_b
    LSR R25             ; desloca mantissa B para direita
    ROR R24
    DEC R20
    RJMP shift_b
end_shift_b:

aligned:
; Agora R30 = sinal_A, R31 = sinal_B
    ; Compare:
    CP R30, R31
    BREQ same_signs; Sinais diferentes - subtração
    ; Determinar qual valor é maior
    CP R23, R25         ; compara byte alto das mantissas
    BRNE high_diff
    CP R22, R24         ; se bytes altos iguais, compara bytes baixos

high_diff:
    BRLO b_greater      ; se A < B

    ; A >= B: R23:R22 - R25:R24
    SUB R22, R24
    SBC R23, R25
    MOV R27, R30        ; sinal do resultado = sinal de A
    RJMP normalize

b_greater:
    ; B > A: R25:R24 - R23:R22
    SUB R24, R22
    SBC R25, R23
    MOV R27, R31        ; sinal do resultado = sinal de B
    ; Troca R25:R24 ? R23:R22
    MOV R22, R24
    MOV R23, R25
    RJMP normalize

same_signs:
    ; Sinais iguais - adição
    ADD R22, R24        ; soma bytes baixos
    ADC R23, R25        ; soma bytes altos com carry
    MOV R27, R30        ; sinal do resultado = sinal de A (ou B, são iguais)

normalize:
    ; Verificar se houve overflow na soma e ajustar
    SBRC R23, 3         ; verifica se bit 3 está ligado (overflow)
    RJMP adjust_overflow

    ; Verificar subflow (mantissa zerada)
    MOV R28, R23
    OR R28, R22         ; R28 = R23 | R22
    BREQ result_zero    ; se resultado for zero

    ; Normalizar o resultado (deslocar à esquerda até bit implícito estar na posição correta)
normalize_loop:
    SBRC R23, 2         ; verifica se bit implícito está na posição
    RJMP end_normalize  ; se sim, fim da normalização
    
    LSL R22             ; desloca mantissa à esquerda
    ROL R23
    DEC R26             ; decrementa expoente
    
    ; Verificar subflow de expoente
    CPI R26, 0
    BRNE normalize_loop
    
    ; Resultado denormalizado
    RJMP end_normalize

adjust_overflow:
    LSR R23             ; desloca à direita para ajustar overflow
    ROR R22
    INC R26             ; incrementa expoente
    
    ; Verificar overflow de expoente (máx 31 = 0x1F)
    CPI R26, 0x1F
    BRNE end_normalize
    
    ; Overflow - definir resultado como infinito
    LDI R23, 0x00
    LDI R22, 0x00
    LDI R26, 0x1F       ; expoente máximo
    RJMP end_normalize

result_zero:
    LDI R26, 0          ; expoente zero
    LDI R27, 0          ; sinal positivo

end_normalize:
    ; Montar resultado final em formato half-precision
    ANDI R23, 0x03      ; manter apenas bits 9-8 da mantissa
    
    ; Adicionar expoente
    LSL R26             ; desloca expoente para posição correta
    LSL R26
    ANDI R26, 0x7C      ; garante que apenas bits de expoente estejam setados
    OR R23, R26         ; adiciona expoente ao byte alto
    
    ; Adicionar bit de sinal
    CPI R27, 1
    BRNE no_sign
    ORI R23, 0x80       ; seta bit de sinal se negativo

no_sign:
   RET

op_sub_16bits:

   ORI R19, 0x80

   CALL op_add_16bits

   RET

program_start:

; === Inicialização da USART0 para 9600 baud ===
    ; Clock = 16 MHz → UBRR = (16*10^6)/(16*9600) - 1 = 103
    LDI R16, 0x67
    STS UBRR0L, R16
    LDI R16, 0x00
    STS UBRR0H, R16

    ; Habilita transmissor
    LDI R16, 0x08
    STS UCSR0B, R16

    ; Modo: assíncrono, 8 bits, sem paridade, 1 stop
    LDI R16, 0x06
    STS UCSR0C, R16

; push half-precision float
    LDI R16, 0x00 ; low byte
    PUSH R16
    LDI R17, 0x47 ; high byte
    PUSH R17

; push half-precision float
    LDI R16, 0x00 ; low byte
    PUSH R16
    LDI R17, 0x45 ; high byte
    PUSH R17

; push half-precision float
    LDI R16, 0x00 ; low byte
    PUSH R16
    LDI R17, 0x3C ; high byte
    PUSH R17

; pop half-precision fields
    POP R17         ; high byte
    POP R16         ; low byte

;  Moving values from op 1
    MOV R18, R16         ; copy low byte op1
    MOV R19, R17         ; copy high byte op1
; pop half-precision fields
    POP R17         ; high byte
    POP R16         ; low byte

   CALL op_add_16bits

   PUSH R22
   PUSH R23

; pop half-precision fields
    POP R17         ; high byte
    POP R16         ; low byte

;  Moving values from op 1
    MOV R18, R16         ; copy low byte op1
    MOV R19, R17         ; copy high byte op1

; pop half-precision fields
    POP R17         ; high byte
    POP R16         ; low byte

   CALL op_sub_16bits

   PUSH R22
   PUSH R23

   POP R23
   POP R22

    ; === Envia resultado (R23:R22) em HEX ===
    MOV R16, R23
    RCALL send_byte_hex

    MOV R16, R22
    RCALL send_byte_hex

    ; envia quebra de linha
    LDI R16, 0x0A
    RCALL send_byte

    RJMP end

; === Função: envia o byte em R16 via serial ===
send_byte:
    ; Espera UDR vazio
wait_udr:
    LDS R17, UCSR0A
    SBRS R17, UDRE0
    RJMP wait_udr

    ; Envia byte
    STS UDR0, R16
    RET

; === Função: envia R16 como 2 dígitos HEX (ASCII) ===
send_byte_hex:
    PUSH R16        
    PUSH R17
    PUSH R18
    PUSH R19

    ; nibble alto
    MOV  R17, R16   ; cópia
    SWAP R17
    ANDI R17, 0x0F
    CPI  R17, 10
    BRLT hex_digit1
    LDI  R19, 'A'
    SUBI R19, 10
    ADD  R17, R19
    RJMP nibble_high_ready

hex_digit1:
    LDI R19, '0'
    ADD R17, R19

nibble_high_ready:
    MOV R16, R17
    RCALL send_byte
    
    ; nibble baixo
    POP R19
    POP R18
    POP R17
    POP R16
    
    PUSH R16
    PUSH R17
    PUSH R18
    PUSH R19
    
    ANDI R16, 0x0F
    CPI  R16, 10
    BRLT hex_digit2
    LDI  R19, 'A'
    SUBI R19, 10
    ADD  R16, R19
    RJMP nibble_low_ready

hex_digit2:
    LDI R19, '0'
    ADD R16, R19

nibble_low_ready:
    RCALL send_byte

    ; Restaura a pilha na ordem inversa
    POP R19
    POP R18
    POP R17
    POP R16
    RET

end:
    RJMP end