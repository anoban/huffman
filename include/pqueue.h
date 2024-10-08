#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
#include <utilities.h>
// clang-format on

// priority que is a data structure derived from heaps.
// priority queue allow us to pick up the next highest priority element from a collection fastly.
// "priority" here is subjective and can mean very different things depending on context.

#define DEFAULT_QUEUE_CAPACITY 1024LLU

typedef struct _pqueue { // priority que
        uint32_t count;
        uint32_t capacity;
        bool (*predptr)(_In_ const void* const, _In_ const void* const);
        void** tree;
} PQueue;

static_assert(sizeof(PQueue) == 24);
static_assert(offsetof(PQueue, count) == 0);
static_assert(offsetof(PQueue, capacity) == 4);
static_assert(offsetof(PQueue, predptr) == 8);
static_assert(offsetof(PQueue, tree) == 16);

// using pascal case for queues
static inline bool __cdecl PQueueInit(
    _Inout_ PQueue* const pqueue, _In_ bool (*const predicate)(_In_ const void* const child, _In_ const void* const parent)
) {
    // predicate must return true whenever a swap is needed.
    // static inline bool __cdecl predicate(_In_ const void* const  child, _In_ const void* const  parent) {
    // retrun child->comparable > parent->comparable ? true : false;
    // }

    pqueue->count    = 0;
    pqueue->capacity = DEFAULT_QUEUE_CAPACITY;
    pqueue->predptr  = predicate;
    // NOLINTNEXTLINE(bugprone-multi-level-implicit-pointer-conversion, bugprone-assignment-in-if-condition
    if (!(pqueue->tree = _CXX_COMPAT_REINTERPRET_CAST(void**, malloc(DEFAULT_QUEUE_CAPACITY * sizeof(uintptr_t))))) return false;
    memset(pqueue->tree, 0U, DEFAULT_QUEUE_CAPACITY * sizeof(uintptr_t));
    return true;
}

static inline void __cdecl PQueueClean(_Inout_ PQueue* const pqueue) {
    for (size_t i = 0; i < pqueue->count; ++i) free(pqueue->tree[i]);
    free(pqueue->tree);
    memset(pqueue, 0U, sizeof(PQueue));
    return;
}

// enqueue
static inline bool __cdecl PQueuePush(_Inout_ PQueue* const pqueue, _In_ const void* const data) {
    void*  tmp      = NULL;
    size_t childpos = 0, parentpos = 0;

    if (pqueue->count + 1 > pqueue->capacity) {
        if (!(tmp = realloc(pqueue->tree, (pqueue->count + DEFAULT_QUEUE_CAPACITY) * sizeof(uintptr_t)))) return false;
        pqueue->tree = _CXX_COMPAT_REINTERPRET_CAST(void**, tmp);
    }

    pqueue->capacity            = pqueue->count + DEFAULT_QUEUE_CAPACITY;
    pqueue->tree[pqueue->count] = _CXX_COMPAT_CONST_CAST(void*, data);
    childpos                    = pqueue->count;
    parentpos                   = parent_position(childpos);

    while ((childpos > 0) && pqueue->predptr(pqueue->tree[childpos], pqueue->tree[parentpos])) {
        tmp                     = pqueue->tree[childpos];
        pqueue->tree[parentpos] = pqueue->tree[childpos];
        pqueue->tree[childpos]  = tmp;
        tmp                     = NULL;
        childpos                = parentpos;
        parentpos               = parent_position(childpos);
    }

    pqueue->count++;
    return true;
}

// dequeue
static inline bool __cdecl PQueuePop(_Inout_ PQueue* const pqueue, _Inout_ void** data) {
    size_t leftchildpos = 0, rightchildpos = 0, parentpos = 0, pos = 0;
    void*  tmp = NULL;

    if (!pqueue->count) return false;

    if (pqueue->count == 1) {
        *data = pqueue->tree[0];
        pqueue->count--;
        PQueueClean(pqueue);
        return true;
    }

    *data                           = pqueue->tree[0];
    pqueue->tree[0]                 = NULL;
    pqueue->tree[0]                 = pqueue->tree[pqueue->count - 1];
    pqueue->tree[pqueue->count - 1] = NULL;
    pqueue->count--;

    while (true) {
        leftchildpos  = lchild_position(parentpos);
        rightchildpos = rchild_position(parentpos);

        if (leftchildpos < pqueue->count && pqueue->predptr(pqueue->tree[leftchildpos], pqueue->tree[parentpos]))
            pos = leftchildpos;
        else
            pos = parentpos;

        if (rightchildpos < pqueue->count && pqueue->predptr(pqueue->tree[rightchildpos], pqueue->tree[pos])) pos = rightchildpos;
        if (pos == parentpos) break;

        tmp                        = pqueue->tree[parentpos];
        pqueue->tree[parentpos]    = pqueue->tree[leftchildpos];
        pqueue->tree[leftchildpos] = tmp;
        tmp                        = NULL;
        parentpos                  = pos;
    }
    return true;
}

static inline void* __stdcall PQueuePeek(_In_ const PQueue* const pqueue) { return !pqueue->tree ? NULL : pqueue->tree[0]; }
