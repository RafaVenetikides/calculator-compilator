//
// Created by Rafael Venetikides on 25/03/25.
//


#include "../../include/utils/float_conversions.h"

uint16_t float2half_rn (float a) {
    uint32_t ia = float_as_uint32 (a);
    uint16_t ir;

    ir = (ia >> 16) & 0x8000;
    if ((ia & 0x7f800000) == 0x7f800000) {
        if ((ia & 0x7fffffff) == 0x7f800000) {
            ir |= 0x7c00; /* infinity */
        } else {
            ir |= 0x7e00 | ((ia >> (24 - 11)) & 0x1ff); /* NaN, quietened */
        }
    } else if ((ia & 0x7f800000) >= 0x33000000) {
        int shift = (int)((ia >> 23) & 0xff) - 127;
        if (shift > 15) {
            ir |= 0x7c00; /* infinity */
        } else {
            ia = (ia & 0x007fffff) | 0x00800000; /* extract mantissa */
            if (shift < -14) { /* denormal */
                ir |= ia >> (-1 - shift);
                ia = ia << (32 - (-1 - shift));
            } else { /* normal */
                ir |= ia >> (24 - 11);
                ia = ia << (32 - (24 - 11));
                ir = ir + ((14 + shift) << 10);
            }
            if ((ia > 0x80000000) || ((ia == 0x80000000) && (ir & 1))) {
                ir++;
            }
        }
    }
    return ir;
}

uint32_t float_as_uint32 (float a) {
    uint32_t r;
    memcpy (&r, &a, sizeof r);
    return r;
}

HalfPrecision unpack_half(uint16_t value) {
    HalfPrecision hp;
    hp.low_byte = value & 0xFF;
    hp.high_byte = (value >> 8) & 0xFF;
    return hp;
}

// Implementation found in: https://stackoverflow.com/questions/76799117/how-to-convert-a-float-to-a-half-type-and-the-other-way-around-in-c
// Adapted to attend to this project's needs