#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      CAUTION                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// offset argument in bit manipulation functions means the literal offset into the stream of bits                    //
// E.G AN OFFSET 3 MEANS THE FOURTH BIT OF THE FIRST BYTE NOT THE BIT AT 2^3 VALUE POSITION OF THE FIRST BYTE!       //
// IN BYTE 0b0100'1000, OFFSET 3 POINTS TO THE BIT INSIDE THE PARENTHESIS 0b010(0)'1000                              //
// NOT THE BIT AT THE POSITION OF 2^3 WHICH IS 0b0100'(1)000                                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// returns the nth bit in the buffer (which is an array of bytes that is viewed as a contiguous stream of bits)
static inline bool __stdcall getbit(
    _In_ const unsigned char* const restrict bitstream, _In_ const unsigned long long offset /* nth bit */
) {
    // const unsigned char byte   = bitstream[offset / 8 /* deliberate integer division */]; // first find the byte that contains the asked bit.
    // const unsigned long long  bit    = offset % 8;                                              // offset of the asked bit within the byte.
    // unsigned char       mask   = 0b1000'0000;
    // for (unsigned long long _ = 0; _ < bit; ++_) mask >>= 1;
    // mask >>= bit;
    assert(bitstream);
    return bitstream[offset / 8] & (0b1000'0000 >> (offset % 8));
}

// toggles a select bit on or off.
static inline void __stdcall setbit(
    _Inout_ unsigned char* const restrict bitstream, _In_ const unsigned long long offset, _In_ const bool flag /* on or off */
) {
    // const unsigned long long bit    = offset % 8;
    // unsigned char      mask   = 0b1000'0000;
    // for (unsigned long long _ = 0; _ < bit; ++_) mask >>= 1;
    // mask              >>= bit;
    assert(bitstream);
    if (flag)
        bitstream[offset / 8] |= 0b1000'0000 >> (offset % 8);
    else
        bitstream[offset / 8] &= ~(0b1000'0000 >> (offset % 8));
}

// computes the bitwise xor of the passed buffers, and stores the result in the output buffer.
static inline void __stdcall xorbit( // the result will be false only if both of the bits are identical
    _In_ const unsigned char* const restrict inbuff_a,
    _In_ const unsigned char* const restrict inbuff_b,
    _Inout_ unsigned char* const restrict outbuff,
    _In_ const unsigned long long offset
) {
    assert(inbuff_a);
    assert(inbuff_b);
    assert(outbuff);
    getbit(inbuff_a, offset) == getbit(inbuff_b, offset) ? setbit(outbuff, offset, false) : setbit(outbuff, offset, true);
}
