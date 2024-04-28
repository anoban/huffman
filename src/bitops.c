#include <huffman.h>

// ALL ROUTINES HAVE BEEN TESTED
// DO NOT REFACTOR WITHOUT RE-TESTING TO ENSURE IMPLEMENTATION INTEGRITY

// get's the nth BIT in the buffer (which is an array of bytes)
// views bitstream as a contiguous stream of bits.
bool __stdcall getBit(_In_ const uint8_t* const restrict bitstream, _In_ const size_t offset /* nth bit */) {
    const uint8_t byte   = bitstream[offset / 8]; // first find the byte that contains the asked bit.
    const size_t  offbit = offset % 8;            // offset of the asked bit within the byte.
    uint8_t       mask   = 0x01;                  // 0000 0001
    for (size_t i = offbit; i > 0; --i) mask <<= 1;
    return byte & mask;
}

// switches a select bit on or off.
void __stdcall setBit(
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
void __stdcall xorBit(
    _In_ const uint8_t* const restrict inbuff_0,
    _In_ const uint8_t* const restrict inbuff_1,
    _Inout_ uint8_t* const restrict outbuff,
    _In_ const size_t offset
) {
    if (getBit(inbuff_0, offset) != getBit(inbuff_1, offset))
        setBit(outbuff, offset, true);
    else
        setBit(outbuff, offset, false);
    return;
}

// quite similar to bit shifts but here the bit that gets pushed off the boundary will be reintroduced into the byte at the other end.
// 10110110 left rotated by 4 bits will be        01101011
// whereas left shifting by 4 bits will result in 00001011
void __stdcall leftRotateBit(
    _Inout_ uint8_t* const restrict bitstream,
    _In_ const size_t length /* length of bitstream in bits */,
    _In_ const size_t n /* rotate n bits left */
) {
    /*
    the goal is to rotate the entire bitstream left by n bits.
    first the leftmost bit of the buffer is stored temporarily and all the bits are shifted one position left
    the temporarily stored bit is assigned to the rightmost bit position.
    Iteratively repeats this n times, admittedly inefficient, an efficient implementation would likely call for an assembly routine or
    intrinsics using SIMD instructions.
    */

    if (!length) return;

    bool leftmost = false,
         pushoff  = false; // leftmost and rightmost bits within the shift boundary i.e bitstream + offset to bitstream + offset + n

    for (size_t _ = 0; _ < n; ++_) {                        // number of bits to rotate
        for (size_t j = 0; j <= (length - 1) / 8LLU; ++j) { // number of bytes in the specified bitstream
            // captures the state of the first bit of the selected byte
            pushoff = getBit(bitstream + j, 0 /* first bit of the current byte */);

            if (!j) // first bit and last bit of the stream need swapping, so store the first bit of first byte.
                leftmost = pushoff;
            else
                // swap the first (leftmost) bit of the current byte with the last (rightmost) bit of the previous byte.
                setBit(bitstream + j - 1 /* picking the previous byte */, 7 /* the last bit */, pushoff);
            bitstream[j] >>= 1; // Intel x86_64 is little endian, so we need to do a right shift for left rotation
        }
        // set the last (rightmost) bit of the rotate window to the first bit of it.
        setBit(bitstream, length - 1, leftmost);
    }

    return;
}
