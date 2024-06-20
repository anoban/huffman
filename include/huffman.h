// master include for all project TUs.

#pragma once
#ifndef __HUFFMAN_H__
    #define __HUFFMAN_H__

    #include <assert.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <stdlib.h>

// clang-format off
    #define _AMD64_                  
    #define WIN32_LEAN_AND_MEAN      
    #define WIN32_EXTRA_MEAN         
    #define __STDC_WANT_SECURE_LIB__ 1
    #define NOMINMAX                 
    
    #include <windef.h>
    #include <errhandlingapi.h>
    #include <fileapi.h>
    #include <handleapi.h>
    #include <sal.h>
// clang-format on

// MSVC headers, for some fucking reason use a plain #if directive for __STDC_WANT_SECURE_LIB__
// not an #ifdef directive, so we need to provide a valued definition for __STDC_WANT_SECURE_LIB__
// a plain #define will results in a compile time error!

typedef struct _node { // represents a Huffman node.
        uint8_t byte;
        size_t  freq;
} node_t;
static_assert(sizeof(node_t) == 16, "");

typedef struct _code { // represents a Huffman code
        bool     is_used;
        uint8_t  length;
        uint16_t code;
} code_t;
static_assert(sizeof(code_t) == 4, "");

typedef struct _heap {     // heap
        uint64_t count;    // number of nodes.
        uint64_t capacity; // number of nodes the heap can hold before requiring a reallocation.
        bool (*fnptr_pred)(_In_reads_(1) const void* const restrict child, _In_reads_(1) const void* const restrict parent);
        void (*fnptr_clean)(_In_reads_(1) const void* const restrict memblock);
        void** tree; // a heap allocated array containing pointers to heap allocated nodes.
                     // use malloc to allocate the tree and the nodes.
} heap_t;
static_assert(sizeof(heap_t) == 40, "");

typedef struct _pque { // priority que
        uint64_t count;
        uint64_t capacity;
        bool (*fnptr_pred)(_In_reads_(1) const void* const restrict child, _In_reads_(1) const void* const restrict parent);
        void (*fnptr_clean)(_In_reads_(1) const void* const restrict memblock);
        void** tree;
} pque_t;
static_assert(sizeof(pque_t) == 40, "");

////////////////////////////
// prototypes :: fileio.c //
////////////////////////////

uint8_t* __cdecl open(_In_ const wchar_t* restrict filepath, _Inout_ unsigned long* const nbytes);

bool __cdecl write(_In_ const wchar_t* const restrict filepath, _In_ const uint8_t* const restrict buffer, _In_ const unsigned long size);

//////////////////////////
// prototypes :: heap.c //
//////////////////////////

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

//////////////////////////
// prototypes :: pque.c //
//////////////////////////

bool pqueInit(
    _Inout_ pque_t* const restrict pque,
    _In_ const bool (*predicate)(_In_reads_(1) const void* const restrict child, _In_reads_(1) const void* const restrict parent),
    _In_ const void (*clean)(_In_reads_(1) const void* const restrict memblock)
);

void pqueClean(_Inout_ pque_t* const restrict pque);

bool pquePush(_Inout_ pque_t* const restrict pque, _In_ const void* const restrict data);

bool pquePop(_Inout_ pque_t* const restrict pque, _Inout_ void** restrict data);

void* __stdcall pquePeek(_In_ const pque_t* const restrict pque);

/////////////////////////////
// prototypes :: huffman.c //
/////////////////////////////

// returns the size of compressed data in bytes if the compression was successfull, -1 if the compression failed.
int64_t compress(_In_ const uint8_t* const restrict inbuffer, _Inout_ uint8_t* restrict outbuffer, _In_ const size_t size);

// returns the size of decompressed data in bytes if the decompression was successfull, -1 if the decompression failed.
int64_t uncompress(_In_ const uint8_t* const restrict inbuffer, _Inout_ uint8_t* restrict outbuffer, _In_ const size_t size);

#endif // !__HUFFMAN_H__