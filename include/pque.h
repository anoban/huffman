/* FOR COMPREHENSIVE IMPLEMENTATION DETAILS SEE heap.h */

#pragma once
#ifndef __PQUE_H__
    #define __PQUE_H__
    #include <stdint.h>
    #include <stdbool.h>
    #include <stdlib.h>

// Priority que is a data structure derived from heaps.
// Priority ques allow us to pick up the next highest priority element from a collection fastly.
// "priority" here is subjective and can mean very different things depending on context.

// Code improvised from Mastering Algorithms with C (1999) Kyle Loudon

    #define HPCAP 1024Ui64

static inline size_t __stdcall parentPos(_In_ const size_t child_pos) { return (child_pos - 1) / 2; } // truncating division.
static inline size_t __stdcall leftChildPos(_In_ const size_t parent_pos) { return (parent_pos * 2) + 1; }
static inline size_t __stdcall rightChildPos(_In_ const size_t parent_pos) { return (parent_pos * 2) + 2; }

typedef struct _pque {
        uint64_t count;
        uint64_t capacity;
        bool     (*fnptr_pred)(_In_reads_(1) const void* const restrict child, _In_reads_(1) const void* const restrict parent);
        void     (*fnptr_clean)(_In_reads_(1) const void* const restrict memblock);
        void**   tree;
} pque_t;

/*
// must return true whenever a swap is needed.
static inline bool __cdecl predicate(
    _In_reads_(1) const void* const restrict child,
    _In_reads_(1) const void* const restrict parent
)
{
    retrun child->comparable > parent->comparable ? true : false;
}
*/

static inline bool pqueInit(
    _Inout_ pque_t* const restrict pque,
    _In_ const bool (*predicate)(_In_reads_(1) const void* const restrict child, _In_reads_(1) const void* const restrict parent),
    _In_ const void (*clean)(_In_reads_(1) const void* const restrict memblock)
) {
    pque->count       = 0;
    pque->capacity    = HPCAP;
    pque->fnptr_pred  = predicate;
    pque->fnptr_clean = clean;
    if (!(pque->tree = malloc(HPCAP * sizeof(uintptr_t)))) return false;
    memset(pque->tree, 0U, HPCAP * sizeof(uintptr_t));
    return true;
}

static inline void pqueClean(_Inout_ pque_t* const restrict pque) {
    for (size_t i = 0; i < pque->count; ++i) free(pque->tree[i]);
    free(pque->tree);
    memset(pque, 0U, sizeof(pque_t));
    return;
}

static inline bool pquePush(_Inout_ pque_t* const restrict pque, _In_ const void* const restrict data) {
    void*  tmp      = NULL;
    size_t childpos = 0, parentpos = 0;

    if (pque->count + 1 > pque->capacity) {
        if (!(tmp = realloc(pque->tree, (pque->count + HPCAP) * sizeof(uintptr_t)))) return false;
        pque->tree = tmp;
    }

    pque->capacity          = pque->count + HPCAP;
    pque->tree[pque->count] = data;
    childpos                = pque->count;
    parentpos               = parentPos(childpos);

    while ((childpos > 0) && pque->fnptr_pred(pque->tree[childpos], pque->tree[parentpos])) {
        tmp                   = pque->tree[childpos];
        pque->tree[parentpos] = pque->tree[childpos];
        pque->tree[childpos]  = tmp;
        tmp                   = NULL;
        childpos              = parentpos;
        parentpos             = parentPos(childpos);
    }

    pque->count++;
    return true;
}

static inline bool pquePop(_Inout_ pque_t* const restrict pque, _Inout_ void** restrict data) {
    size_t leftchildpos = 0, rightchildpos = 0, parentpos = 0, pos = 0;
    void*  tmp = NULL;

    if (!pque->count) return false;

    if (pque->count == 1) {
        *data = pque->tree[0];
        pque->count--;
        pqueClean(pque);
        return true;
    }

    *data                       = pque->tree[0];
    pque->tree[0]               = NULL;
    pque->tree[0]               = pque->tree[pque->count - 1];
    pque->tree[pque->count - 1] = NULL;
    pque->count--;

    while (true) {
        leftchildpos  = leftChildPos(parentpos);
        rightchildpos = rightChildPos(parentpos);

        if (leftchildpos < pque->count && pque->fnptr_pred(pque->tree[leftchildpos], pque->tree[parentpos]))
            pos = leftchildpos;
        else
            pos = parentpos;

        if (rightchildpos < pque->count && pque->fnptr_pred(pque->tree[rightchildpos], pque->tree[pos])) pos = rightchildpos;
        if (pos == parentpos) break;

        tmp                      = pque->tree[parentpos];
        pque->tree[parentpos]    = pque->tree[leftchildpos];
        pque->tree[leftchildpos] = tmp;
        tmp                      = NULL;
        parentpos                = pos;
    }
    return true;
}

static inline const void* __stdcall pquePeek(_In_ const pque_t* const restrict pque) { return pque->tree == NULL ? NULL : pque->tree[0]; }

#endif // !__PQUE_H__