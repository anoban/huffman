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

static inline bool __cdecl heap_init(
    _Inout_ heap_t* const restrict heap, _In_ bool (*const predicate)(_In_ const void* const, _In_ const void* const)
) {
    assert(heap);
    assert(predicate);

    // NOLINTNEXTLINE(bugprone-multi-level-implicit-pointer-conversion)
    heap->tree = _CXX_COMPAT_REINTERPRET_CAST(void**, malloc(DEFAULT_HEAP_CAPACITY_BYTES));

    if (!heap->tree) {
        fwprintf_s(stderr, L"memory allocation error inside %s @LINE: %d\n", __FUNCTIONW__, __LINE__);
        return false;
    }

    heap->count    = 0;
    heap->capacity = DEFAULT_HEAP_CAPACITY;
    heap->predptr  = predicate;

    memset(heap->tree, 0U, DEFAULT_HEAP_CAPACITY_BYTES); // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    return true;
}

static inline void __cdecl heap_clean(_Inout_ heap_t* const restrict heap) {
    assert(heap);

    for (unsigned i = 0; i < heap->count; ++i) {
        free(heap->tree[i]);
        // heap->tree[i] = NULL;   // redundant
    }
    free(heap->tree); // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    memset(heap, 0U, sizeof(heap_t));
}

static inline bool __cdecl heap_push(
    _Inout_ heap_t* const restrict heap, _In_ const void* const restrict pushed

) {
    assert(heap);
    assert(pushed);

    void * _temp_node = NULL, **_temp_tree = NULL; // NOLINT(readability-isolate-declaration)
    size_t _childpos = 0, _parentpos = 0;          // NOLINT(readability-isolate-declaration)

    if (heap->count + 1 > heap->capacity) { // if the current buffer doesn't have space for another pointer,

        // NOLINTNEXTLINE(bugprone-assignment-in-if-condition, bugprone-multi-level-implicit-pointer-conversion)
        if (!(_temp_tree = _CXX_COMPAT_REINTERPRET_CAST(
                  void**, realloc(heap->tree, heap->capacity * sizeof(uintptr_t) + DEFAULT_HEAP_CAPACITY_BYTES)
              ))) {
            fwprintf_s(stderr, L"memory reallocation error inside %s @LINE: %d\n", __FUNCTIONW__, __LINE__);
            return false;
        }

        heap->tree      = _temp_tree;
        heap->capacity += DEFAULT_HEAP_CAPACITY;
    }

    
    heap->count++;                                                       // increment the node count.
    heap->tree[heap->count - 1] = _CXX_COMPAT_CONST_CAST(void*, pushed); // hook in the new node

    _childpos                   = heap->count - 1;            // offset of the newly inserted node.
    _parentpos                  = parent_position(_childpos); // offset of the new node's parent.

    while ((_childpos > 0) /* as long as we haven't reached the root node at offset 0 */ &&
           (*heap->predptr)(
               heap->tree[_childpos], heap->tree[_parentpos]
           ) /* and the weight of the child is greater than that of the parent */) {
        _temp_node             = heap->tree[_childpos];  // child node to be pushed up
        heap->tree[_childpos]  = heap->tree[_parentpos]; // push the parent node down
        heap->tree[_parentpos] = _temp_node;             // push the child node up
        _temp_tree             = NULL;

        // prepare for the next swap in anticipation that the new positioning of the inserted node also needs a swap.
        _childpos              = _parentpos;                 // after the swap, parent's offset becomes child's offset.
        _parentpos             = parent_position(_childpos); // find the parent of the current child's offset.
    }

    return true;
}

static inline bool __cdecl heap_pop(_Inout_ heap_t* const restrict heap, _Inout_ void** const restrict popped /* popped out pointer */) {
    assert(heap);
    assert(popped);

    size_t _leftchildpos = 0, _rightchildpos = 0, _parentpos = 0, _pos = 0; // NOLINT(readability-isolate-declaration)
    void*  _temp = NULL;

    if (!heap->count) { // if the heap is empty,
        *popped = NULL;
        return false;
    }

    if (heap->count == 1) { // if the heap has only one node,
        *popped     = heap->tree[0];
        heap->count = 0;
        heap_clean(heap); // since heap->count is 0, the looped free()s inside heap_clean() won't be executed.
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
