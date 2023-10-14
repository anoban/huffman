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
        node_t   nodes[MAX_TOTAL_NODES]; // `an array of 256 node_t s.`
        uint64_t freqs[256];             // `an array holding the frequencies of each byte in the file.`
        uint8_t  orderedbytes[256];      // `bytes sorted by ascending frequency.`
        uint64_t nleaves;                // `number of leaf nodes in the tree.`
        uint64_t nlinks;                 // `number of link nodes in the tree.`
        bool     isfreqtable_initialized;
} huffman_t;
#pragma pack(pop)

// We won't be hooking up nodes without children to parents, so those variants need not to be here.
typedef enum LINK {
    NPSC = -3, // `No Parent, Single Child`
    NPTC,      // `No Parent, Two Children`
    PTC,       // `Parent, Two Children`
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

void static inline __stdcall initialize_frequency_table(
    _Inout_ huffman_t* const restrict tree, _In_ const uint8_t* const restrict bytestream, _In_ const uint64_t length
) {
    // byte = bytestream[i]
    // freqs[byte]++
    for (uint64_t i = 0; i < length; ++i) tree->freqs[bytestream[i]]++;
    tree->isfreqtable_initialized = true;
    return;
}

// Say that our input contains all possible bytes at frequency > 1.
// We'll then need 256 terminal nodes.
// Each linking node will need 2 child nodes.
// Total number of nodes (maximum possible) is 2N - 1 i.e = (256 * 2) - 1 = 511
// Since we are willing to use 256 terminal nodes (again, we will never ever need nodes more than this. Sort of an overkill, but okay.)
// The number of linker nodes will be 2N - 1 - N = (N - 1) i.e 255

// Sort the bytes array, using the frequencies of bytes in the byte stream as reference, and returns the offset of the first byte with
// non-zero frequency. Uses UCRT's qsort internally. Caller does not need to initialize the `bytes` array.
uint64_t static inline __stdcall sort_bytes(
    _Inout_ huffman_t* restrict const tree, _In_ _CoreCrtSecureSearchSortCompareFunction predicate
) {
    if (!tree->isfreqtable_initialized) {
        fwprintf_s(
            stderr,
            L"Error: (Line %d, File %s) The frequency table of the passed Huffman tree hasn't been initialized.\n",
            __LINE__,
            __FILEW__
        );
        return 0;
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
    return pos;
}

int wmain(void) {
    uint64_t       nbytes   = 0;
    const uint8_t* contents = open(L"moby_dick.txt", &nbytes);

    huffman_t      hftree;

    initialize_frequency_table(&hftree, contents, nbytes);

    // The bytes with higher frequencies need the lowest number of bits for encoding in order to make the space use minimal.
    // Bytes that occur rarely like the non-printing characters in a text file, can be represented by comparatively larger number of bits,
    // since we won't be using them that often.

    // Traversing right in the Huffman tree, each edge bears a label 1
    // Traversing left in the Huffman tree, each edge bears a label 0

    // We can ignore all the bytes with frequency 0.

    uint8_t  sortedbytes[256]; // Bytes sorted by frequency. (ascending order)
    // The offset of the first byte in the sorted array, with non-zero frequency.
    uint64_t firstnzbytepos = sort_bytes(sortedbytes, frequencies, predicate);

    for (uint64_t i = 0; i < 256; ++i) printf_s("%d (%c): %llu\n", sortedbytes[i], sortedbytes[i], frequencies[sortedbytes[i]]);
    printf_s(
        "First non-zero offset is %llu and the byte with the lowest non-zero frequency is %c\n", firstnzbytepos, sortedbytes[firstnzbytepos]
    );

    node_t huffman_tree[MAX_TOTAL_NODES]; // This will be our Huffman tree, Just a plain array of node_t s on the stack.
    for (size_t i = 0; i < MAX_TOTAL_NODES; ++i) {
        huffman_tree[i].left = huffman_tree[i].right = NULL;
        huffman_tree[i].weight                       = -1000;
        huffman_tree[i].byte                         = 0;
    }

    // 256 - firstnzbytepos will give the number of needed leaf nodes.
    const uint64_t n_leafnodes = 256 - firstnzbytepos; // Equal to the number of bytes with non-zero frequencies.

    // First N nodes in huffman_tree will be leaf nodes.
    // Initialize the leaf nodes. Works good :)

    uint64_t       writecaret  = 0; // A caret to track the offset of the last initialized node in the huffman tree (node_t array).
    for (uint64_t i = firstnzbytepos; /* Start from the first byte with non-zero frequency */ i < 256; ++i) {
        huffman_tree[writecaret].byte   = sortedbytes[i];
        huffman_tree[writecaret].weight = frequencies[sortedbytes[i]];
        huffman_tree[writecaret].left = huffman_tree[writecaret].right = NULL;
        // Since leaf nodes have nothing pointed by their arms, make these pointers NULL.
        writecaret++;
    }

    // Now writecaret must be N
    printf_s("%llu leaf nodes have been initialized\n", writecaret);

    // Creating linker nodes, nodes with larger weights should be attached to the right arm.

    uint64_t n_constructedlinks     = 0;                 // Number of linker nodes that have been constructed so far.
    uint64_t n_neededlinks          = (n_leafnodes - 1); // = N - 1
    uint64_t n_linkedleaves         = 0;                 // Number of leaf nodes that have been linked to a linker node.
    int64_t  readcaret              = 0;                 // This caret needs to traverse through leaf nodes and link nodes.

    // In the linking process, only the first link could be a trivial link between the first two leaf nodes.
    // All the following linkings will require us to find the next node to be linked by finding the un-linked node with the lowest
    // frequency.

    // N + 1 th node in the array & 1st linker node.
    huffman_tree[writecaret].left   = &huffman_tree[0]; // First node needs to be linked with the left arm of the linker node.
    huffman_tree[writecaret].right  = &huffman_tree[1]; // Second node needs to be linked with the right arm of the linker node.
    huffman_tree[writecaret].byte   = NPTC;
    huffman_tree[writecaret].weight = huffman_tree[0].weight + huffman_tree[1].weight;

    n_constructedlinks++;                               // First link node done.
    writecaret++;
    n_linkedleaves += 2;
    readcaret      += 2;                                // We've already read the first two leaf nodes. Don't worry about them anymore.

    // From all the nodes we have, find the two with the lowest frequencies, excluding the nodes that have been linked already.

    while (n_neededlinks != n_constructedlinks) {
        // If we had N leaf nodes in the tree, make the following nodes linker nodes. i.e starting from the offset N.
        // Lower weighted linkers should be in the left.

        uint64_t min_consolidated_weight = 0;
        node_t*  next                    = NULL;

        // Scanning through the nodes.
        for (uint64_t i = 0; i < (2 * n_leafnodes) - 1 /* Nnumber of total nodes */; ++i) {
            // The smallest cumulative weight in the tree, so far.
            min_consolidated_weight = huffman_tree[writecaret - 1].weight;

            // If there are nodes (including leaf nodes) with weights smaller than or equal to the min_consolidated_weight;
            if (huffman_tree[i].weight <= min_consolidated_weight) {
                // Create a now link node, linking it to the unlinked node with smallest weight.
                huffman_tree[writecaret].byte = NPSC;
                min_consolidated_weight = huffman_tree[writecaret].weight = min_consolidated_weight + huffman_tree[i].weight;
                huffman_tree[writecaret].left                             = &huffman_tree[writecaret - 1];
                huffman_tree[writecaret].right                            = NULL;
                huffman_tree[writecaret - 1].byte                         = PTC; // Register that the previous node now has a parent.

                n_constructedlinks++;
            } else {
                node_t* current = &huffman_tree[writecaret - 1];                 // Find the node with next smallest weight.
                for (uint64_t i = 0; i < writecaret; ++i)
                    if (current->weight < huffman_tree[i].weight) current = &huffman_tree[i];

                huffman_tree[writecaret].byte = NPSC;
                huffman_tree[writecaret].weight =
            }
        }
    }

    return 0;
}
