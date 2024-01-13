#pragma once
#ifndef __HUFFMAN_H__
    #define __HUFFMAN_H__
    #include <stdbool.h>
    #include <stdint.h>

// we don't really need to be this frugal about the memory alignment here :(
// a struct representing a Huffman node.
typedef struct _node {
        uint8_t byte;
        size_t  freq;
} node_t;

// a struct representing a Huffman code
    #pragma pack(push, 2)
typedef struct _code {
        bool     is_used;
        uint8_t  length;
        uint16_t code;
} code_t;
    #pragma pack(pop)

/* Prototypes */

// returns the size of compressed data in bytes if the compression was successfull, -1 if the compression failed.
int64_t compress(_In_ const uint8_t* const restrict inbuffer, _Inout_ uint8_t* restrict outbuffer, _In_ const size_t size);

// returns the size of decompressed data in bytes if the decompression was successfull, -1 if the decompression failed.
int64_t uncompress(_In_ const uint8_t* const restrict inbuffer, _Inout_ uint8_t* restrict outbuffer, _In_ const size_t size);

#endif // !__HUFFMAN_H__