#pragma once
#ifndef __HEAP_H__
    #define __HEAP_H__
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>

#define HPCAP 1024LLU

// Many problems involve a programme to quickly determine the smallest or largest element from a collection
// capable of frequent insertions and deletions.
// One way to address this need is to keep the collection sorted all the time (ascending or descending order).
// However, sorting a collection at every insertion or deletion is expensive.

// But this requirement does not imply that all the elements in a collection need to be sorted.
// It's enough for the collection to know the location of the smallest or the largest element, for fast access.
// Heaps and priority ques are the adept data structures for this kind of functionalities.


// Heaps are organized trees (usually a binary tree) that allow fast access to the largest/smallest element.
// Costs of preserving a tree is negligible compared to the performance penalty incurred by maintaining the container
// sorted all the time.

// In a heap each child node has a weight smaller than its parent's.
// Therefore, it is the root node, that has the largest weight.
// Binary trees like these are top heavy, as we climb up the strata nodes are positioned in, nodes will become
// more and more heavy.

// Trees can also be designed to hold the heaviest nodes at the bottom and lightest nodes at the top, whereby
// the root node will be the one with the smallest weight.
// Under such scenaries, parent node will have a smaller weight compared to its children.
// These trees are bottom heavy.

// Heaps are typically left balanced. When new nodes are annexed to the tree, the tree grows from right to left.

// An efficient way to implement left-balanced binary trees is to store the nodes in a contiguous array.
// Nodes are arranged in this array in the order we'd encounter them in a level traversal.


typedef struct heap {
        uint64_t count;     // number of nodes.
        uint64_t capacity;  // number of nodes the heap can hold before requiring a reallocation.
        int32_t  (*predicate)(_In_reads_(1) const void* const restrict _this, _In_reads_(1) const void* const restrict _next);
        void     (*clean)(_In_reads_(1) const void* const restrict memblock);
        void**   tree;      // a heap allocated array containing pointers to heap allocated nodes.
                            // use malloc to allocate the tree and the nodes.
}heap_t;

static void inline heapInit(
    _In_ heap_t* const restrict heap,
    _In_ const int32_t(*fnptr_pred)(_In_reads_(1) const void* const restrict _this, _In_reads_(1) const void* const restrict _next),
    _In_ const void    (*fnptr_clean)(_In_reads_(1) const void* const restrict memblock)
) {
    heap->count     = 0;
    heap->capacity  = HPCAP;
    heap->predicate = fnptr_pred;
    heap->clean     = fnptr_clean;
    heap->tree      = NULL;
    return;
}

static void inline heapClean(_In_ heap_t* const restrict heap) {
    for (size_t i = 0; i < heap->count; ++i) free(heap->tree[i]); // free the heap allocated nodes.
    free(heap->tree);   // free the array containing pointers to heap allocated nodes.
    // two levels of heap allocation happens here!
    memset(heap,0U, sizeof(heap_t));
    return;
}

static bool inline heapPush(_In_ heap_t* const restrict heap, _In_ const void* const restrict data /* expects a heap allocated memory block */) {
    void*  tmp    = NULL;
    size_t curpos = 0, prevpos = 0;

    // if the buffer doesn't have space for another pointer,
    if (heap->count + 1 >= heap->capacity) {
        // ask for an additional 1024 * sizeof(uintptr_t) bytes. return false if the reallocation has failed.
        if (!(tmp = realloc(heap->tree, (heap->count + HPCAP) * sizeof(uintptr_t)))) return false;
        heap->tree = tmp;
    }
    
    // This is terribly inefficient, genuinely fancying an arena allocator here!
    heap->tree[heap->count] = data;
}


#endif // !__HEAP_H__