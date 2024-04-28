// master include for all project TUs.

#pragma once
#ifndef __HUFFMAN_H__
    #define __HUFFMAN_H__

    #if defined(DEBUG) || defined(_DEBUG)
        #define __RUN_TESTS__
    #endif // (DEBUG) || defined(_DEBUG)

    #include <assert.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <stdlib.h>

    #ifdef _WIN32
        #define _AMD64_                  // architecture
        #define WIN32_LEAN_AND_MEAN      1
        #define WIN32_EXTRA_MEAN         1
        #define __STDC_WANT_SECURE_LIB__ 1
    #endif

    #include <errhandlingapi.h>
    #include <fileapi.h>
    #include <handleapi.h>
    #include <sal.h>

// MSVC headers, for some fucking reason use a plain #if directive for __STDC_WANT_SECURE_LIB__
// not an #ifdef directive, so we need to provide a valued definition for __STDC_WANT_SECURE_LIB__
// a plain #define will results in a compile time error!

    #pragma comment(lib, "User32.lib")

// we don't really need to be this frugal about the memory alignment here :(

typedef struct _node { // represents a Huffman node.
        uint8_t byte;
        size_t  freq;
} node_t;

    #pragma pack(push, 4)
typedef struct _code { // represents a Huffman code
        bool     is_used;
        uint8_t  length;
        uint16_t code;
} code_t;
    #pragma pack(pop)

typedef struct _heap {     // heap
        uint64_t count;    // number of nodes.
        uint64_t capacity; // number of nodes the heap can hold before requiring a reallocation.
        bool (*fnptr_pred)(_In_reads_(1) const void* const restrict child, _In_reads_(1) const void* const restrict parent);
        void (*fnptr_clean)(_In_reads_(1) const void* const restrict memblock);
        void** tree; // a heap allocated array containing pointers to heap allocated nodes.
                     // use malloc to allocate the tree and the nodes.
} heap_t;

typedef struct _pque { // priority que
        uint64_t count;
        uint64_t capacity;
        bool (*fnptr_pred)(_In_reads_(1) const void* const restrict child, _In_reads_(1) const void* const restrict parent);
        void (*fnptr_clean)(_In_reads_(1) const void* const restrict memblock);
        void** tree;
} pque_t;

// prototypes :: bitops.c

bool __stdcall getBit(_In_ const uint8_t* const restrict bitstream, _In_ const size_t offset /* nth bit */);

void __stdcall setBit(
    _Inout_ uint8_t* const restrict bitstream, _In_ const size_t offset, _In_ const bool flag /* on or off */
);

void __stdcall xorBit(
    _In_ const uint8_t* const restrict inbuff_0,
    _In_ const uint8_t* const restrict inbuff_1,
    _Inout_ uint8_t* const restrict outbuff,
    _In_ const size_t offset
);

void __stdcall leftRotateBit(
    _Inout_ uint8_t* const restrict bitstream,
    _In_ const size_t length /* length of bitstream in bits */,
    _In_ const size_t n /* rotate n bits left */
);

// prototypes :: fileio.c

uint8_t* openFile(_In_ const wchar_t* restrict file_name, _Inout_ size_t* const nread_bytes);

bool writeFile(_In_ const wchar_t* const restrict file_path, _In_ const uint8_t* const restrict buffer, _In_ const size_t size);

// prototypes :: heap.c

size_t __stdcall parentPos(_In_ const size_t child_pos);

size_t __stdcall leftChildPos(_In_ const size_t parent_pos);

size_t __stdcall rightChildPos(_In_ const size_t parent_pos);

bool heapInit(
    _Inout_ heap_t* const restrict heap,
    _In_ const bool (*predicate)(_In_reads_(1) const void* const restrict child, _In_reads_(1) const void* const restrict parent),
    _In_ const void (*clean)(_In_reads_(1) const void* const restrict memblock)
);

void heapClean(_Inout_ heap_t* const restrict heap);

bool heapPush(
    _Inout_ heap_t* const restrict heap, _In_ const void* const restrict data /* expects a heap allocated memory block (node) to push in */
);

bool heapPop(_Inout_ heap_t* const restrict heap, _Inout_ void** restrict data /* popped out */);

// prototypes :: pque.c

bool pqueInit(
    _Inout_ pque_t* const restrict pque,
    _In_ const bool (*predicate)(_In_reads_(1) const void* const restrict child, _In_reads_(1) const void* const restrict parent),
    _In_ const void (*clean)(_In_reads_(1) const void* const restrict memblock)
);

void pqueClean(_Inout_ pque_t* const restrict pque);

bool pquePush(_Inout_ pque_t* const restrict pque, _In_ const void* const restrict data);

bool pquePop(_Inout_ pque_t* const restrict pque, _Inout_ void** restrict data);

void* __stdcall pquePeek(_In_ const pque_t* const restrict pque);

// prototypes :: huffman.c

// returns the size of compressed data in bytes if the compression was successfull, -1 if the compression failed.
int64_t compress(_In_ const uint8_t* const restrict inbuffer, _Inout_ uint8_t* restrict outbuffer, _In_ const size_t size);

// returns the size of decompressed data in bytes if the decompression was successfull, -1 if the decompression failed.
int64_t uncompress(_In_ const uint8_t* const restrict inbuffer, _Inout_ uint8_t* restrict outbuffer, _In_ const size_t size);

#endif // !__HUFFMAN_H__