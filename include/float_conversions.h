//
// Created by Rafael Venetikides on 25/03/25.
//

#ifndef FLOAT_CONVERSIONS_H
#define FLOAT_CONVERSIONS_H

#include <stdint.h>
#include <string.h>

typedef struct {
    uint8_t high_byte;
    uint8_t low_byte;
} HalfPrecision;

uint16_t float2half_rn (float a);
uint32_t float_as_uint32 (float a);
HalfPrecision unpack_half(uint16_t value);

#endif //FLOAT_CONVERSIONS_H
