#pragma once

#include <bitops.h>
#include <fileio.h>
#include <heap.h>
#include <pqueue.h>

typedef struct _node { // represents a Huffman node.
        uint32_t byte; // could work with an unsigned char, but will wind up wasting 3 bytes in padding so why not use that space
        uint32_t freq;
} node_t;

static_assert(sizeof(node_t) == 8);
static_assert(offsetof(node_t, byte) == 0);
// weird but static_assert(!offsetof(node_t, byte)); errs with MSVC, but works with gcc, clang & icx
static_assert(offsetof(node_t, freq) == 4);

typedef struct _code { // represents a Huffman code
        bool     is_used;
        uint8_t  length;
        uint16_t code;
} code_t; // no padding yeehaw :)

static_assert(sizeof(code_t) == 4);
static_assert(offsetof(code_t, is_used) == 0);
static_assert(offsetof(code_t, length) == 1);
static_assert(offsetof(code_t, code) == 2);
