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
[[nodiscard, msvc::forceinline]] static __forceinline bool __stdcall getbit(
    _In_ const uint8_t* const restrict bitstream, _In_ const size_t offset /* nth bit */
) {
    // const uint8_t byte   = bitstream[offset / 8 /* deliberate integer division */]; // first find the byte that contains the asked bit.
    // const size_t  bit    = offset % 8;                                              // offset of the asked bit within the byte.
    // uint8_t       mask   = 0b1000'0000;
    // for (size_t _ = 0; _ < bit; ++_) mask >>= 1;
    // mask >>= bit;
    assert(bitstream);
    return bitstream[offset / 8] & (0b1000'0000 >> (offset % 8));
}

// toggles a select bit on or off.
[[msvc::forceinline]] static __forceinline void __stdcall setbit(
    _Inout_ uint8_t* const restrict bitstream, _In_ const size_t offset, _In_ const bool flag /* on or off */
) {
    // const size_t bit    = offset % 8;
    // uint8_t      mask   = 0b1000'0000;
    // for (size_t _ = 0; _ < bit; ++_) mask >>= 1;
    // mask              >>= bit;
    assert(bitstream);
    if (flag)
        bitstream[offset / 8] |= 0b1000'0000 >> (offset % 8);
    else
        bitstream[offset / 8] &= ~(0b1000'0000 >> (offset % 8));
}

// computes the bitwise xor of the passed buffers, and stores the result in the output buffer.
static __forceinline void __stdcall xorbit( // the result will be false only if both of the bits are identical
    _In_ const uint8_t* const restrict inbuff_a,
    _In_ const uint8_t* const restrict inbuff_b,
    _Inout_ uint8_t* const restrict outbuff,
    _In_ const size_t offset
) {
    assert(inbuff_a);
    assert(inbuff_b);
    assert(outbuff);
    [[msvc::forceinline_calls]] getbit(inbuff_a, offset) == getbit(inbuff_b, offset) ? setbit(outbuff, offset, false) :
                                                                                       setbit(outbuff, offset, true);
}
