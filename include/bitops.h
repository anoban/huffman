#pragma once
#ifndef __BITOPS_H__
    #define __BITOPS_H__
#include <stdbool.h>
#include <stdint.h>

// get's the nth BIT in the buffer (which is an array of bytes)
// views bitstream as a contiguous stream of bits.
static inline bool getbit(_Inout_ const uint8_t* const restrict bitstream, _In_ const size_t offset) {
    const uint8_t byte   = bitstream[offset / 8]; // first find the byte that contains the asked bit.
    const size_t  offbit = offset % 8;            // offset of the asked bit within the byte.
    uint8_t       mask   = 0x01;                  // 0000 0001
    for (size_t i = offbit; i > 0; --i) mask <<= 1;
    return byte & mask;
}

// switches a select bit on or off.
static inline void setbit(_Inout_ uint8_t* const restrict bitstream, _In_ const size_t offset, _In_ const bool flag) {
    const size_t  offbit = offset % 8;            
    uint8_t       mask   = 0x01;                  // 0000 0001
    for (size_t i = offbit; i > 0; --i) mask <<= 1;
    if (flag) bitstream[offset / 8] |= mask;
    else bitstream[offset / 8] &= (~mask);
    return;
}

// computes the bitwise xor of the passed buffers, and stores the result in the output buffer.
static inline void xorbit(
    _In_ const uint8_t* const restrict inbuff_0, _In_ const uint8_t* const restrict inbuff_1,
    _Inout_ uint8_t* const restrict outbuff, _In_ const size_t offset
) {
    if (getbit(inbuff_0, offset) != getbit(inbuff_1, offset)) { 
        setbit(outbuff, offset, true);
    } else {
        setbit(outbuff, offset, false);
    }
    return;
}


#endif // !__BITOPS_H__
