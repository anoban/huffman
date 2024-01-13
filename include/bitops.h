#pragma once
#ifndef __BITOPS_H__
    #define __BITOPS_H__
    #include <stdbool.h>
    #include <stdint.h>

// get's the nth BIT in the buffer (which is an array of bytes)
// views bitstream as a contiguous stream of bits.
static inline bool __stdcall getbit(_In_ const uint8_t* const restrict bitstream, _In_ const size_t offset /* nth bit */) {
    const uint8_t byte   = bitstream[offset / 8]; // first find the byte that contains the asked bit.
    const size_t  offbit = offset % 8;            // offset of the asked bit within the byte.
    uint8_t       mask   = 0x01;                  // 0000 0001
    for (size_t i = offbit; i > 0; --i) mask <<= 1;
    return byte & mask;
}

// switches a select bit on or off.
static inline void __stdcall setbit(
    _Inout_ uint8_t* const restrict bitstream, _In_ const size_t offset, _In_ const bool flag /* on or off */
) {
    const size_t offbit = offset % 8;
    uint8_t      mask   = 0x01;                                                               // 0000 0001
    for (size_t i = offbit; i > 0; --i) mask <<= 1;
    if (flag)
        bitstream[offset / 8] |= mask;
    else
        bitstream[offset / 8] &= (~mask);
    return;
}

// computes the bitwise xor of the passed buffers, and stores the result in the output buffer.
static inline void __stdcall xorbit(
    _In_ const uint8_t* const restrict inbuff_0,
    _In_ const uint8_t* const restrict inbuff_1,
    _Inout_ uint8_t* const restrict outbuff,
    _In_ const size_t offset
) {
    if (getbit(inbuff_0, offset) != getbit(inbuff_1, offset))
        setbit(outbuff, offset, true);
    else
        setbit(outbuff, offset, false);
    return;
}

// quite similar to bit shifts but here the bit that gets pushed off the boundary will be reintroduced into the byte at the other end.
// 10110110 left rotated by 4 bits will be        01101011
// whereas left shifting by 4 bits will result in 00001011
static inline void __stdcall leftrotbit(
    _Inout_ uint8_t* const restrict bitstream, _In_ const size_t offset, _In_ const size_t n /* rotate n bits left */
) {
    /* first stores the leftmost bit of the buffer temporarily, and shifts all the bits left and assigns the temporarily stored bit to the
    rightmost position. Iteratively repeats this n times, admittedly inefficient, an efficient implementation would likely call for an
    assembly routine or intrinsics using SIMD instructions */

    bool leftmost = false, rightmost = false;
    if (!offset) return;

    for (size_t i = 0; i < n; ++i) {                        // number of bits to rotate
        for (size_t j = 0; j <= (offset - 1) / 8LLU; ++j) { // number of bytes in the specified bitstream
            leftmost = getbit(bitstream + j, 0 /* first bit of the current byte */);
            if (!j)
                rightmost = leftmost;
            else
                setbit(bitstream + j - 1 /* picking the previous byte */, 7 /* the last bit */, leftmost);
            bitstream[j] <<= bitstream[j];
        };
    }

    return;
}
#endif // !__BITOPS_H__
