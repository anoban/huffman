#pragma once
#ifndef __HUFFMAN_H__
    #define __HUFFMAN_H__

// clang-format off
    #define _AMD64_                  
    #define WIN32_LEAN_AND_MEAN      
    #define WIN32_EXTRA_MEAN          
    // UCRT headers, for some freaking reason, use a plain #if predicate for __STDC_WANT_SECURE_LIB__
    // not an #ifdef predicate, so we need to provide a valued definition for __STDC_WANT_SECURE_LIB__,
    // a plain #define results in compile time error
    #define __STDC_WANT_SECURE_LIB__ 1
    #define NOMINMAX                 

    #include <windef.h>
    #include <errhandlingapi.h>
    #include <fileapi.h>
    #include <handleapi.h>
    #include <sal.h>    // for gcc
// clang-format on

    #include <assert.h>
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <stdlib.h>

// clang-format off
    #include <bitops.h>
// clang-format on

typedef struct _node { // represents a Huffman node.
        uint8_t  byte;
        uint32_t freq;
} node_t;

static_assert(sizeof(node_t) == 8, "struct node_t must be 8 bytes in size!");
static_assert(
    offsetof(node_t, byte) == 0, "member 'byte' must be aligned at the struct boundary!"
); // weird but static_assert(!offsetof(node_t, byte), ""); errs with MSVC, but works with gcc, clang & icx
static_assert(offsetof(node_t, freq) == 4, "member 'freq' must be aligned 4 bytes away from the struct boundary!");

typedef struct _code { // represents a Huffman code
        bool     is_used;
        uint8_t  length;
        uint16_t code;
} code_t;

static_assert(sizeof(code_t) == 4, "struct code_t must be 4 bytes in size!");
static_assert(offsetof(code_t, is_used) == 0, "member 'is_used' must be aligned at the struct boundary!");
static_assert(offsetof(code_t, length) == 1, "member 'length' must be aligned 1 bytes away from the struct boundary!");
static_assert(offsetof(code_t, code) == 2, "member 'code' must be aligned 2 bytes away from the struct boundary!");

typedef struct _heap {     // heap
        uint32_t count;    // number of nodes.
        uint32_t capacity; // number of nodes the heap can hold before requiring a reallocation.
        bool (*fnptr_pred)(_In_ const void* const restrict, _In_ const void* const restrict);
        void** tree; // a heap allocated array containing pointers to heap allocated nodes.
                     // use malloc to allocate the tree and the nodes.
} heap_t;

static_assert(sizeof(heap_t) == 24, "struct heap_t must be 32 bytes in size!");
static_assert(offsetof(heap_t, count) == 0, "member 'count' must be aligned at the struct boundary!");
static_assert(offsetof(heap_t, capacity) == 4, "member 'capacity' must be aligned 4 bytes away from the struct boundary!");
static_assert(offsetof(heap_t, fnptr_pred) == 8, "member 'fnptr_pred' must be aligned 8 bytes away from the struct boundary!");
static_assert(offsetof(heap_t, tree) == 16, "member 'tree' must be aligned 16 bytes away from the struct boundary!");

typedef struct _pque { // priority que
        uint32_t count;
        uint32_t capacity;
        bool (*fnptr_pred)(_In_ const void* const restrict, _In_ const void* const restrict);
        void** tree;
} pque_t;

static_assert(sizeof(pque_t) == 24, "struct pque_t must be 32 bytes in size!");
static_assert(offsetof(pque_t, count) == 0, "member 'count' must be aligned at the struct boundary!");
static_assert(offsetof(pque_t, capacity) == 4, "member 'capacity' must be aligned 4 bytes away from the struct boundary!");
static_assert(offsetof(pque_t, fnptr_pred) == 8, "member 'fnptr_pred' must be aligned 8 bytes away from the struct boundary!");
static_assert(offsetof(pque_t, tree) == 16, "member 'tree' must be aligned 16 bytes away from the struct boundary!");

static __forceinline uint32_t __stdcall get_parent(_In_ const uint32_t cpos) {
    return (cpos - 1) / 2 /* deliberate truncating division. */;
}

static __forceinline uint32_t __stdcall get_leftchild(_In_ const uint32_t ppos) { return ppos * 2 + 1; }

static __forceinline uint32_t __stdcall get_rightchild(_In_ const uint32_t ppos) { return ppos * 2 + 2; }

    #pragma region __FILEIO_PROTOTYPES__
uint8_t* __cdecl open(_In_ const wchar_t* restrict filepath, _Inout_ unsigned long* const nbytes);

bool __cdecl write(_In_ const wchar_t* const restrict filepath, _In_ const uint8_t* const restrict buffer, _In_ const unsigned long size);
    #pragma endregion

    #pragma region __HEAP_PROTOTYPES__
bool heap_init(
    _Inout_ heap_t* const restrict heap, _In_ const bool (*predicate)(_In_ const void* const restrict, _In_ const void* const restrict)
);

void heap_clean(_Inout_ heap_t* const restrict heap);

bool heap_push(
    _Inout_ heap_t* const restrict heap, _In_ const void* const restrict data /* expects a heap allocated memory block (node) to push in */
);

bool heap_pop(_Inout_ heap_t* const restrict heap, _Inout_ void** restrict data /* popped out */);
    #pragma endregion

    #pragma region __PQUE_PROTOTYPES__
bool pqueInit(
    _Inout_ pque_t* const restrict pque,
    _In_ const bool (*predicate)(_In_reads_(1) const void* const restrict child, _In_reads_(1) const void* const restrict parent),
    _In_ const void (*clean)(_In_reads_(1) const void* const restrict memblock)
);

void pqueClean(_Inout_ pque_t* const restrict pque);

bool pquePush(_Inout_ pque_t* const restrict pque, _In_ const void* const restrict data);

bool pquePop(_Inout_ pque_t* const restrict pque, _Inout_ void** restrict data);

void* __stdcall pquePeek(_In_ const pque_t* const restrict pque);
    #pragma endregion

    #pragma region __HUFFMAN_PROTOTYPES__
int64_t compress(_In_ const uint8_t* const restrict inbuffer, _Inout_ uint8_t* restrict outbuffer, _In_ const size_t size);

int64_t uncompress(_In_ const uint8_t* const restrict inbuffer, _Inout_ uint8_t* restrict outbuffer, _In_ const size_t size);
    #pragma endregion
#endif // !__HUFFMAN_H__