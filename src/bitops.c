#include <huffman.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      CAUTION                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// offset argument in bit manipulation functions means the literal offset into the stream of bits                    //
// E.G AN OFFSET 3 MEANS THE FOURTH BIT OF THE FIRST BYTE NOT THE BYTE AT 2^3 VALUE POSITION OF THE FIRST BYTE!      //
// IN BYTE 0b0100'1000, OFFSET 3 POINTS TO THE BIT INSIDE THE PARENTHESIS 0b010(0)'1000                              //
// NOT THE BIT AT THE POSITION OF 2^3 WHICH IS 0b0100'(1)000                                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// get's the nth bit in the buffer (which is an array of bytes) which is viewed as a contiguous stream of bits.
bool __stdcall getbit(_In_ const register uint8_t* const restrict bitstream, _In_ const register size_t offset /* nth bit */) {
    const uint8_t byte   = bitstream[offset / 8 /* deliberate integer division */]; // first find the byte that contains the asked bit.
    const size_t  offbit = offset % 8;                                              // offset of the asked bit within the byte.
    uint8_t       mask   = 0b1000'0000;
    for (size_t i = offbit; i > 0; --i) mask >>= 1;
    return byte & mask;
}

// switches a select bit on or off.
void __stdcall setbit(
    _Inout_ register uint8_t* const restrict bitstream, _In_ const size_t offset, _In_ const register bool flag /* on or off */
) {
    const size_t offbit = offset % 8;
    uint8_t      mask   = 0b0000'0001;
    for (size_t i = offbit; i > 0; --i) mask <<= 1;
    if (flag) // set bit true
        bitstream[offset / 8] |= mask;
    else // set bit false
        bitstream[offset / 8] &= (~mask);
    return;
}

// computes the bitwise xor of the passed buffers, and stores the result in the output buffer.
void __stdcall xorbit(
    _In_ const register uint8_t* const restrict ibuff_a,
    _In_ const register uint8_t* const restrict ibuff_b,
    _Inout_ register uint8_t* const restrict obuff,
    _In_ const register size_t offset
) {
    if (getbit(ibuff_a, offset) != getbit(ibuff_b, offset))
        setbit(obuff, offset, true);
    else
        setbit(obuff, offset, false);
    return;
}

// quite similar to bit shifts but here the bit that gets pushed off the boundary will be reintroduced into the byte at the other end.
// 10110110 left rotated by 4 bits will be        01101011 whereas left shifting by 4 bits will result in 00001011
void __stdcall lrotbits(
    _Inout_ register uint8_t* const restrict bitstream,
    _In_ register const size_t length, // length of bitstream in bits
    _In_ register const size_t n       // rotate n bits left
) {
    // the goal is to rotate the entire bitstream left by n bits.
    // first the leftmost bit of the buffer is stored temporarily and all the bits are shifted one position left
    // the temporarily stored bit is assigned to the rightmost bit position.
    if (!length) return;

    bool leftmost = false, pushoff = false;
    // leftmost and rightmost bits within the shift boundary i.e bitstream + offset to bitstream + offset + n

    for (size_t _ = 0; _ < n; ++_) {                        // number of bits to rotate
        for (size_t j = 0; j <= (length - 1) / 8LLU; ++j) { // number of bytes in the specified bitstream
            // captures the state of the first bit of the selected byte
            pushoff = getbit(bitstream + j, 0); // first bit of the current byte

            if (!j) // first bit and last bit of the stream need swapping, so store the first bit of first byte.
                leftmost = pushoff;
            else
                // swap the first (leftmost) bit of the current byte with the last (rightmost) bit of the previous byte.
                setbit(bitstream + j - 1 /* picking the previous byte */, 7 /* the last bit */, pushoff);
            bitstream[j] >>= 1; // Intel x86_64 is little endian, so we need to do a right shift for left rotation
        }
        // set the last (rightmost) bit of the rotate window to the first bit of it.
        setbit(bitstream, length - 1, leftmost);
    }

    return;
}
