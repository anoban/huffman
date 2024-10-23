#pragma once
#include <utilities.h>

#define DEFAULT_HEAP_CAPACITY       1024LLU
#define DEFAULT_HEAP_CAPACITY_BYTES (DEFAULT_HEAP_CAPACITY * sizeof(uintptr_t))

typedef struct _heap { // heap
        uint32_t count;
        uint32_t capacity;
        bool (*predptr)(_In_ const void* const, _In_ const void* const);
        void** tree;
} heap_t;

static_assert(sizeof(heap_t) == 24);
static_assert(offsetof(heap_t, count) == 0);
static_assert(offsetof(heap_t, capacity) == 4);
static_assert(offsetof(heap_t, predptr) == 8);
static_assert(offsetof(heap_t, tree) == 16);

static inline bool __cdecl heap_pop(_Inout_ heap_t* const restrict heap, _Inout_ void** const restrict popped) {
    assert(heap);
    assert(popped);

    size_t _leftchildpos = 0, _rightchildpos = 0, _parentpos = 0, _pos = 0; // NOLINT(readability-isolate-declaration)
    void*  _temp = NULL;

    if (!heap->count) { // if the heap is empty,
        *popped = NULL;
        return false;
    }

    if (heap->count == 1) {
        *popped     = heap->tree[0];
        heap->count = 0;
        heap_clean(heap);
        return true;
    }

    // {25, 20, 24, 17, 19, 22, 12, 15, 7, 9, 18, 10}
    // give up the node at the top (root node)
    *popped                     = heap->tree[0];
    // heap->tree[0]               = NULL;
    // {NULL, 20, 24, 17, 19, 22, 12, 15, 7, 9, 18, 10}

    /* now the tree looks like this:

                             (NULL)                <--- 3rd (Root/Top)
                             /     \
                            /       \
                           /         \
                          /           \
                         /             \
                        /               \
                      (20)              (24)       <--- 2nd
                     /    \             /  \
                    /      \           /    \
                  (17)     (19)      (22)  (12)    <--- 1st
                  /  \     /  \      /
                 /    \   /    \    /
               (15)   (7)(9)  (18)(10)             <--- 0th (Bottom)
    */

    // move the last node in the array to the root's position, offset 0.
    heap->tree[0]               = heap->tree[heap->count - 1];
    heap->tree[heap->count - 1] = NULL;
    heap->count--;

    /* now the tree looks like this:

                               (10)                <--- 3rd (Root/Top)
                             /     \
                            /       \
                           /         \
                          /           \
                         /             \
                        /               \
                      (20)              (24)       <--- 2nd
                     /    \             /  \
                    /      \           /    \
                  (17)     (19)      (22)  (12)    <--- 1st
                  /  \     /  \
                 /    \   /    \
               (15)   (7)(9)  (18)                 <--- 0th (Bottom)
    */
    // {10, 20, 24, 17, 19, 22, 12, 15, 7, 9, 18}

    assert(!_parentpos); // at the begininng of the while loop, _parentpos must be 0
    while (true) {       // rearrange the tree
        _leftchildpos  = lchild_position(_parentpos);
        _rightchildpos = rchild_position(_parentpos);

        // unless we are at the end of the array and the weight of the left child is greater than that of the parent
        _pos = (_leftchildpos <= (heap->count - 1)) && (*heap->predptr)(heap->tree[_leftchildpos], heap->tree[_parentpos]) ? _leftchildpos :
                                                                                                                             _parentpos;
        // choose to traverse down the left arm of the node, otherwise hold the caret at the parent.
        // in our example, this block will choose the left arm.

        // unless we are at the end of the array and if the weight of the right child is greater than that of the parent OR the left child (CONTEXT DEPENDENT)
        if ((_rightchildpos <= (heap->count - 1)) && (*heap->predptr)(heap->tree[_rightchildpos], heap->tree[_pos])) _pos = _rightchildpos;
        // choose to traverse down the right arm
        // in our example this conditional will choose the right arm, because 24 > 10.

        // in our example, both conditionals at line 315 and 321 will evaluate to true.
        // since the expressions predicted on the conditional at line 321 will be evaluated finally,
        // it is the right child that will be swapped with the parent first.

        // if the root is heavier than both the left and right children, no need to rearrange the heap.
        if (_pos == _parentpos) break;

        _temp                  = heap->tree[_parentpos];
        heap->tree[_parentpos] = heap->tree[_pos];
        heap->tree[_pos]       = _temp;

        /* now the tree looks like this:

                          (24)                <--- 3rd (Root/Top)
                        /     \
                       /       \
                      /         \
                     /           \
                    /             \
                   /               \
                 (20)              (10)       <--- 2nd
                /    \             /  \
               /      \           /    \
             (17)     (19)      (22)  (12)    <--- 1st
             /  \     /  \
            /    \   /    \
          (15)   (7)(9)  (18)                 <--- 0th (Bottom)
       */

        // move down one level. i.e the current right child becomes the parent node for the next iteration.
        _parentpos             = _pos;
    }

    /* at the start of second iteration, the tree looks like this:

                           (24)                <--- 3rd (Root/Top)
                         /     \
                        /       \
                       /         \
                      /           \
                     /             \
                    /               \
                  (20)              (10)       <--- 2nd
                 /    \             /  \
                /      \           /    \
              (17)     (19)      (22)  (12)    <--- 1st
              /  \     /  \
             /    \   /    \
           (15)   (7)(9)  (18)                 <--- 0th (Bottom)
    */
    // again the first conditional will pick the left arm for swapping as 22 > 10.
    // second conditional won't execute as 22 < 12, finally the left arm will be selected for swapping.

    /* at the end of second iteration, the tree looks like this:

                           (24)                <--- 3rd (Root/Top)
                         /     \
                        /       \
                       /         \
                      /           \
                     /             \
                    /               \
                  (20)              (22)       <--- 2nd
                 /    \             /  \
                /      \           /    \
              (17)     (19)      (12)  (10)    <--- 1st
              /  \     /  \
             /    \   /    \
           (15)   (7)(9)  (18)                 <--- 0th (Bottom)
     */
    // no rearrangements needed anymore.

    return true;
}
