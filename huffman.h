#pragma once

#ifndef __HUFFMAN__
    #define __HUFFMAN__

    #include <stdbool.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    #ifdef _WIN32
        #define _AMD64_
        #define WIN32_LEAN_AND_MEAN
        #define WIN32_EXTRA_MEAN
    #endif

    #include <errhandlingapi.h>
    #include <fileapi.h>
    #include <handleapi.h>

    #define MAX_TOTAL_NODES 511LLU
    #define MAX_LEAF_NODES  256LLU
    #define MAX_LINK_NODES  255LLU

// Self referential structs need a forward declaration since when the self reference is analyzed by the compiler, it won't be able to
// figure out its type, since the type is used inside its declaration.

static inline uint8_t* open(_In_ const wchar_t* restrict file_name, _Out_ uint64_t* const nread_bytes) {
    *nread_bytes    = 0;
    void *   handle = NULL, *buffer = NULL;
    uint32_t nbytes = 0;

    handle          = CreateFileW(file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

    if (handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER file_size;
        if (!GetFileSizeEx(handle, &file_size)) {
            fwprintf_s(stderr, L"Error %lu in GetFileSizeEx\n", GetLastError());
            return NULL;
        }

        // add an extra megabyte to the buffer, for safety.
        size_t buffsize = file_size.QuadPart + (1024U * 1024);

        // caller is responsible for freeing this buffer.
        buffer          = malloc(buffsize);
        if (buffer) {
            if (ReadFile(handle, buffer, buffsize, &nbytes, NULL)) {
                CloseHandle(handle);
                *nread_bytes = nbytes;
                return buffer;
            } else {
                fwprintf_s(stderr, L"Error %lu in ReadFile\n", GetLastError());
                CloseHandle(handle);
                free(buffer);
                return NULL;
            }
        } else {
            fputws(L"Memory allocation error: malloc returned NULL", stderr);
            CloseHandle(handle);
            return NULL;
        }
    } else {
        fwprintf_s(stderr, L"Error %lu in CreateFileW\n", GetLastError());
        return NULL;
    }
}

// There are two ways to workaround this problem:
// 1) typedef struct node_t node_t;
// 2) Or one could annotate the self reference as a struct.

    #pragma pack(push, 8)
typedef struct node_t {
        struct node_t* left;  // Zero
        struct node_t* right; // One
        int64_t  byte;   // This serves a cocktail of purposes, besides registering the byte for leaf nodes. Intentionally using a 64 bit
                         // signed integer.
                         // Positive values in the range 0 to 255 mark a leaf node.
                         // Negative values: Lookup enum LINKNODETYPE.
                         // Takes up 8 bytes, so will likely eliminate the need for padding bytes too.
        uint64_t weight; // Frequency in the case of leaf nodes (nodes that directly represent bytes)
                         // Sum of the frequencies of child nodes, in the case of linking nodes.
} node_t;
    #pragma pack(pop)

    #pragma pack(push, 8)
typedef struct huffman_t {
        node_t   nodes[MAX_TOTAL_NODES]; // `an array of 511 node_t s, sorted by weight`
        uint64_t freqs[256];             // `an array holding the frequencies of each byte in the file.`
        uint8_t  orderedbytes[256];      // `bytes sorted by ascending frequency.`
        uint64_t nleaves;                // `number of leaf nodes in the tree.`
        uint64_t nlinks;                 // `number of link nodes in the tree.`
        bool     isfreqtable_initialized;
        bool     is_initialized;
} huffman_t;
    #pragma pack(pop)

// We won't be hooking up nodes without two children to parents, so those variants need not to be here.
typedef enum LINK_ {
    NPNC = -100, // `has no parent, has no children`
    NPC,         // `has no parent, has a child`
    NPCC,        // `has no parent, has two children`
    PCC,         // `has a parent, has two children`
} LINK;

// context must be the predicate's first argument, function signature must be in the following form:
// `compare(context, (void*) &elem1, (void*) &elem2)`;
// designed to sort in descending order, if ascending sort is desired, just swap the return values -1 and 1.
int32_t static inline __cdecl predicate(
    _In_reads_(1) void* restrict context, _In_reads_(1) const void* restrict this, _In_reads_(1) const void* restrict next
) {
    const uint64_t* frequencies = context; // aliasing a restricted pinter :(
    uint8_t         __this = *((uint8_t*) this), __next = *((uint8_t*) next);
    if (frequencies[__this] == frequencies[__next])
        return 0;
    else if (frequencies[__this] > frequencies[__next])
        return 1;
    else
        return -1; // frequencies[__this] < frequencies[__next]
}

static void inline init_huffmantree(_Inout_ huffman_t* const restrict tree) {
    for (size_t i = 0; i < 256; ++i) tree->freqs[i] = tree->orderedbytes[i] = 0;

    tree->nleaves = tree->nlinks = 0;
    for (size_t i = 0; i < MAX_TOTAL_NODES; ++i) {
        tree->nodes[i].byte   = NPNC;
        tree->nodes[i].weight = 0;
        tree->nodes[i].left = tree->nodes[i].left = NULL;
    }
    tree->is_initialized          = true;
    tree->isfreqtable_initialized = false;
    return;
}

static void inline __stdcall init_freqtable(
    _Inout_ huffman_t* const restrict tree, _In_ const uint8_t* const restrict bytestream, _In_ const uint64_t length
) {
    if (!tree->is_initialized) {
        fwprintf_s(stderr, L"Error: (Line %d, File %s) Passed Huffman tree is uninitialized.\n", __LINE__, __FILEW__);
        return;
    }

    for (uint64_t i = 0; i < length; ++i) tree->freqs[bytestream[i]]++;
    tree->isfreqtable_initialized = true;
    return;
}

// Say that our input contains all possible bytes at frequency > 1.
// We'll then need 256 terminal nodes.
// Each linking node will need 2 child nodes.
// Total number of nodes (maximum possible) is 2N - 1 i.e = (256 * 2) - 1 = 511
// Since we are willing to use 256 leaf nodes (again, we will never ever need leaf nodes more than this.
// The number of linker nodes will be 2N - 1 - N = (N - 1) i.e 255

// Sort the bytes array, using the frequencies of bytes in the byte stream as reference, and returns the offset of the first byte with
// non-zero frequency. Uses UCRT's qsort internally. Caller does not need to initialize the `bytes` array.
static void inline __stdcall sort_bytes(_Inout_ huffman_t* restrict const tree, _In_ _CoreCrtSecureSearchSortCompareFunction predicate) {
    if (!tree->isfreqtable_initialized) {
        fwprintf_s(
            stderr,
            L"Error: (Line %d, File %s) The frequency table of the passed Huffman tree hasn't been initialized.\n",
            __LINE__,
            __FILEW__
        );
        return;
    }

    for (size_t i = 0; i < 256; ++i) tree->orderedbytes[i] = (uint8_t) i; // Initialize the array with bytes 0 to 255.
    qsort_s(tree->orderedbytes, 256U, 1U, predicate, tree->freqs);

    // Find the first offset of the byte in orderedbytes, with the lowest non-zero frequency.
    size_t pos = 0;
    for (size_t i = 0; i < 256; ++i) {
        if (tree->freqs[tree->orderedbytes[i]] > 0) { // orderedbytes have been sorted by ascending frequency order.
            pos = i;
            break;
        }
    }
    tree->nleaves = 256 - pos;                        // not 255 - pos!
    tree->nlinks  = tree->nleaves - 1;                // N - 1
    return;
}

static void inline init_leaves(_In_ huffman_t* const restrict tree) {
    if (!tree->is_initialized || !tree->isfreqtable_initialized) {
        fwprintf_s(
            stderr, L"Error: (Line %d, File %s) The frequency table and/or Huffman tree has/ven't been initialized.\n", __LINE__, __FILEW__
        );
        return;
    }

    const size_t nskips = 256 - tree->nleaves;
    for (size_t i = 0; i < tree->nleaves; ++i) {
        tree->nodes[i].byte   = tree->orderedbytes[nskips + i]; // cherry picking only the bytes with non-zero frequencies.
        tree->nodes[i].weight = tree->freqs[tree->nodes[i].byte];
        tree->nodes[i].left = tree->nodes[i].right = NULL;      // no branches for leaf nodes.
    }
    return;
}

static void inline printbytes(_In_ const huffman_t* const restrict tree) {
    for (size_t i = 0; i < 256; ++i) wprintf_s(L"%4zu) %3d: %10llu\n", i, tree->orderedbytes[i], tree->freqs[tree->orderedbytes[i]]);
    return;
}

typedef struct nodepair_ {
        node_t* left;
        node_t* right;
} nodepair_t;

// scans the nodes array for a pair of un-hooked (no parent) nodes with the lowest frequencies.
static nodepair_t inline scan_nodes(
    _In_ const huffman_t* const restrict tree,
    _In_ const size_t   ignore /* offsets to ignore */,
    _In_ const uint64_t cutoff /* weights need to be greater than this  */
) {
    // check the node type.
    // if it's a link node, check for its current status.
    // check if the weight is less than or equal to the cutoff.

    nodepair_t pair   = { NULL, NULL };
    uint64_t   weight = cutoff, offset = 0;
    size_t     i = ignore;

    for (; i < MAX_TOTAL_NODES; ++i) {
        if (tree->nodes[i].byte != PCC && tree->nodes[i].weight >= cutoff) {
            weight = tree->nodes[i].weight; // register the weight
            break;
        }
    }
}

// when creating new nodes, always make sure they are arranged in the ascending order of their weights, regardless of the node type.
static void inline create_links(_Inout_ huffman_t* const restrict tree) { }

#endif //!__HUFFMAN__