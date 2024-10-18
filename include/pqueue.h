#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
#include <utilities.h>
// clang-format on

#define DEFAULT_PQUEUE_CAPACITY       1024LLU
#define DEFAULT_PQUEUE_CAPACITY_BYTES (DEFAULT_PQUEUE_CAPACITY * sizeof(uintptr_t))

typedef struct _pqueue { // priority queue
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
    assert(pqueue);
    assert(predicate);

    pqueue->tree = _CXX_COMPAT_REINTERPRET_CAST( // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
        void**,
        malloc(DEFAULT_PQUEUE_CAPACITY_BYTES)
    );

    if (!pqueue->tree) {
        fwprintf_s(stderr, L"memory allocation error inside %s @LINE: %d\n", __FUNCTIONW__, __LINE__);
        return false;
    }

    pqueue->count    = 0;
    pqueue->capacity = DEFAULT_PQUEUE_CAPACITY;
    pqueue->predptr  = predicate;
    memset(pqueue->tree, 0U, DEFAULT_PQUEUE_CAPACITY_BYTES); // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    return true;
}

static inline void __cdecl PQueueClean(_Inout_ PQueue* const pqueue) {
    for (size_t i = 0; i < pqueue->count; ++i) free(pqueue->tree[i]);
    free(pqueue->tree); // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    memset(pqueue, 0U, sizeof(PQueue));
}

// enqueue
static inline bool __cdecl PQueuePush(_Inout_ PQueue* const pqueue, _In_ const void* const data) {
    assert(pqueue);
    assert(data);

    void * _temp_node = NULL, **_temp_tree = NULL; // NOLINT(readability-isolate-declaration)
    size_t _childpos = 0, _parentpos = 0;          // NOLINT(readability-isolate-declaration)

    if (pqueue->count + 1 > pqueue->capacity) {
        // NOLINTNEXTLINE(bugprone-assignment-in-if-condition, bugprone-multi-level-implicit-pointer-conversion)
        if (!(_temp_tree = _CXX_COMPAT_REINTERPRET_CAST(
                  void**, realloc(pqueue->tree, pqueue->capacity * sizeof(uintptr_t) + DEFAULT_PQUEUE_CAPACITY_BYTES)
              ))) {
            fwprintf_s(stderr, L"memory reallocation error inside %s @LINE: %d\n", __FUNCTIONW__, __LINE__);
            return false;
        }
        pqueue->tree      = _temp_tree;
        pqueue->capacity += DEFAULT_PQUEUE_CAPACITY;
    }

    pqueue->count++;
    pqueue->tree[pqueue->count - 1] = _CXX_COMPAT_CONST_CAST(void*, data);
    _childpos                       = pqueue->count - 1;
    _parentpos                      = parent_position(_childpos);

    while ((_childpos > 0) && (*pqueue->predptr)(pqueue->tree[_childpos], pqueue->tree[_parentpos])) {
        _temp_node               = pqueue->tree[_childpos];
        pqueue->tree[_childpos]  = pqueue->tree[_parentpos];
        pqueue->tree[_parentpos] = _temp_node;
        _temp_tree               = NULL;
        _childpos                = _parentpos;
        _parentpos               = parent_position(_childpos);
    }

    return true;
}

// dequeue
static inline bool __cdecl PQueuePop(_Inout_ PQueue* const pqueue, _Inout_ void** data) {
    assert(pqueue);
    assert(data);

    size_t _leftchildpos = 0, _rightchildpos = 0, _parentpos = 0, _pos = 0; // NOLINT(readability-isolate-declaration)
    void*  _temp = NULL;

    if (!pqueue->count) {
        *data = NULL;
        return false;
    }

    if (pqueue->count == 1) {
        *data         = pqueue->tree[0];
        pqueue->count = 0;
        PQueueClean(pqueue);
        return true;
    }

    *data                           = pqueue->tree[0];

    pqueue->tree[0]                 = pqueue->tree[pqueue->count - 1];
    pqueue->tree[pqueue->count - 1] = NULL;
    pqueue->count--;

    assert(!_parentpos);
    while (true) {
        _leftchildpos  = lchild_position(_parentpos);
        _rightchildpos = rchild_position(_parentpos);

        _pos = (_leftchildpos <= (pqueue->count - 1)) && (*pqueue->predptr)(pqueue->tree[_leftchildpos], pqueue->tree[_parentpos]) ?
                   _leftchildpos :
                   _parentpos;

        if ((_rightchildpos <= (pqueue->count - 1)) && (*pqueue->predptr)(pqueue->tree[_rightchildpos], pqueue->tree[_pos]))
            _pos = _rightchildpos;
        if (_pos == _parentpos) break;

        _temp                    = pqueue->tree[_parentpos];
        pqueue->tree[_parentpos] = pqueue->tree[_pos];
        pqueue->tree[_pos]       = _temp;
        _parentpos               = _pos;
    }

    return true;
}

static inline void* __stdcall PQueuePeek(_In_ const PQueue* const pqueue) { return !pqueue->tree ? NULL : pqueue->tree[0]; }
