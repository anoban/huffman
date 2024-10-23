#pragma once
#include <utilities.h>

typedef union _symbol {
        uint8_t byte; // only applicable to leaf nodes
        // could use a smaller type for marker but the member freq inside hnode_t will warrant padding, so why not use that here
        size_t  marker; // a value (UINT32_MAX) registering a non-leaf node
} symbol_t;

static_assert(sizeof(symbol_t) == 8);
static_assert(!offsetof(symbol_t, byte));
static_assert(!offsetof(symbol_t, marker));

typedef struct _hnode {  // represents a Huffman node.
        symbol_t symbol; // byte for a leaf node, UINT32_MAX for internodes
        size_t   freq;
} hnode_t;

static_assert(sizeof(hnode_t) == 16);
static_assert(!offsetof(hnode_t, symbol));
static_assert(offsetof(hnode_t, freq) == 8);

typedef struct _hcode { // represents a Huffman code
        bool     is_used;
        uint8_t  length;
        uint16_t code;
} hcode_t; // no padding yeehaw :)

static_assert(sizeof(hcode_t) == 4);
static_assert(!offsetof(hcode_t, is_used));
static_assert(offsetof(hcode_t, length) == 1);
static_assert(offsetof(hcode_t, code) == 2);

// the goal of Huffman encoding is to represent symbols that occur more frequently with fewer bits than the symbols that occur less
// frequently - a concept known as minimum entropy coding
// the "symbol" here can be anything but is usually a byte!

// entropy E of a symbol S is defined as E(S) = -log2(P(S))
// where P is the probability of the select symbol being found in the given buffer
// e.g. "ABCBCBCJKUGRFCCCSYJIOIHICCC"
// frequency of the character 'C' in the above string is 9
// total number of characters in that string is 27
// P('C') = 9/27
// E('C') = -log2(P(9/27))
//        = 1.58496250072116    - this is the entropy of the character 'C' in the given string
// in theory, we can respresent 'C' in this string using 1.58496250072116 bits instead of the conventional 8 bits
// since the character 'C' occurs 9 times, the its cumulative contribution to the string's entropy is
// = 9 x 1.58496250072116
// = 14.2646625064904
// again, in theory all the 'C' characters in the above string can be represented by a total of 14.2646625064904 bits

#define FREQ_ARRAY_SIZE 256LLU

static __forceinline bool __cdecl EnumerateBytes(
    _In_ const uint8_t* const restrict inbuffer,
    _Inout_count_(FREQ_ARRAY_SIZE) size_t* const restrict frequencies, // could be a stack based or heap allocated array
    _In_ const size_t size
) {
    assert(buffer);
    assert(size);
    // assumes `frequencies` to be a zeroed out array
    memset(frequencies, 0U, sizeof(typeof(*frequencies)) * (FREQ_ARRAY_SIZE));
    for (size_t i = 0; i < size; ++i) frequencies[inbuffer[i]]++;
}

static inline int64_t __cdecl Compress(
    _In_ const uint8_t* const restrict inbuffer, _Inout_ uint8_t* const restrict outbuffer, _In_ const size_t size
) {
    //
}

static inline int64_t __cdecl Decompress(
    _In_ const uint8_t* const restrict inbuffer, _Inout_ uint8_t* const restrict outbuffer, _In_ const size_t size
) {
    //
}