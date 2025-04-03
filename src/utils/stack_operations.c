//
// Created by Rafael Venetikides on 25/03/25.
//

#include "../../include/utils/float_conversions.h"
#include "../../include/utils/stack_operations.h"

/**
 * @brief Pushes a 16-bit value to the hardware stack
 *
 *  Here we push a 16-bit value to the hardware stack
 *  the value is separated in 3 parts: sign, exponent and mantissa
 *  HalfPrecision is a struct that holds these 3 parts
 *
 * @param val Value to be converted
 * @param outAsm Pointer to the output file
 */
void gen_push_16bit(float val, FILE *outAsm) {

    uint16_t val16 = float2half_rn(val);
    HalfPrecision hp = unpack_half(val16);

    fprintf(outAsm, "; push half-precision float\n"
    "    LDI R16, 0x%02X ; low byte\n"
    "    PUSH R16\n"
    "    LDI R17, 0x%02X ; high byte\n"
    "    PUSH R17\n\n", hp.low_byte, hp.high_byte);
}

/**
 * @brief Pops a 16-bit value from the hardware stack
 *
 * Here we pop a 16-bit value from the hardware stack
 * the value is separated in 3 parts: sign, exponent and mantissa
 * R16 holds the sign
 * R17 holds the exponent
 * R18 holds the mantissa high bits
 * R19 holds the mantissa low byte
 *
 * @param outAsm Pointer to the output file
 */
void gen_pop_16bit(FILE *outAsm) {
    fprintf(outAsm, "; pop half-precision fields\n"
    "    POP R17         ; high byte\n"
    "    POP R16         ; low byte\n\n");
}