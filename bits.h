#pragma once

#ifndef __HUFFMAN__
    #define __HUFFMAN__
    #include <stdbool.h>
    #include <stdint.h>

    #pragma pack(push, 1)
typedef struct HuffNode_ {
        uint8_t  symbol;
        uint64_t frequency;
} HuffNode;

typedef struct HuffCode_ {
        uint8_t  used;
        uint16_t code;
        uint8_t  size;
} HuffCode;
    #pragma pack(pop)

// tested, works fine :)
static inline bool getbit(_In_ const uint8_t* restrict bitstream, _In_ const uint64_t bitpos) {
    uint8_t byte = bitstream[bitpos / 8]; // bitpos = 432541, then the byte will be 54067.625 truncated to 54067.
    uint8_t bit  = bitpos % 8;            // the position of the bit within the select byte.
    uint8_t mask = 0x01;                  // 0000 0001

    // say our byte is 0xA4; 1010 0100 and I want the 5th bit.
    // create a mask in which only the nth bit is set to true, and then do a bitwise and.
    for (size_t i = 0; i < bit; ++i) mask <<= 1;

    // the mask will now be 0000 1000
    //   1010 0100
    // & 0000 1000
    // = 0000 0000
    return byte & mask;
}

static void setbit(_In_ uint8_t* const restrict bitstream, _In_ const uint64_t bitpos, _In_ const bool flag) {
    uint8_t bit = bitpos % 8, mask = 0x01;
    for (size_t i = 0; i < bit; ++i) mask <<= 1;
    if (flag)
        bitstream[bitpos / 8] |= mask;
    else
        bitstream[bitpos / 8] &= (~mask);
    return;
}

#endif // __HUFFMAN__