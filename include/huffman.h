#pragma once
#ifndef __HUFFMAN_H__
    #define __HUFFMAN_H__
    #include <stdint.h>
    #include <stdbool.h>

// A structure representing a Huffman node.
typedef struct _node {
        uint8_t byte;
        size_t  freq;
} node_t;

// A structure representing a Huffman code
    #pragma pack(push, 1)
typedef struct _code {
        bool     is_used;
        uint8_t  length;
        uint16_t code;
} code_t;
    #pragma pack(pop)

#endif // !__HUFFMAN_H__