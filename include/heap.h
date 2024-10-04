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

#define DEFAULT_HEAP_CAPACITY       1024LLU
#define DEFAULT_HEAP_CAPACITY_BYTES (DEFAULT_HEAP_CAPACITY * sizeof(uintptr_t))

// many problems demand determination of the smallest or largest element from a collection that's capable of frequent insertions and deletions
// one way to meet this need is to keep the collection sorted all the time but sorting a collection at every insertion or deletion is expensive

// fortunately this requirement does not insinuate that all the elements in a collection need to be sorted.
// it's enough for the collection to just know the location of the smallest or the largest element, enabling fast access.
// heaps and priority queue are adept for this purpose

// heaps are organized trees (usually a binary tree) that allow easy access to the largest / smallest element.
// the cost of creating and maintaining a tree is negligible compared to the performance overhead incurred by maintaining the container sorted all the time

// in a heap, each child node has a weight smaller than its parent's, ergo it is the root node that has the largest weight in a heap
// binary trees like these are top heavy, as we climb up the hierarchy of nodes, nodes will become more and more heavy (will have a larger weight)

// trees can also be architectured to hold the heaviest nodes at the bottom and lightest nodes at the top, whereby the root node will be the one with the smallest weight
// in that case, parent node will have a smaller weight compared to its children, hence making the tree bottom heavy

// heaps are typically left balanced.
// when new nodes are annexed to the tree at a given level, the tree grows from left to right.

// an efficient way to implement left-balanced binary tree is to store the nodes in a contiguous array.
// nodes are arranged in this array in the order we'd encounter them in a level traversal.

// say that we have 11 leaf nodes with weights 15, 7, 9, 18, 10, 12, 17, 19, 20, 22 ans 25 to begin with.
// NOTE: parent nodes here aren't created from the children!
// THIS TREE ISN'T REPRESENTATIVE OF AN ORDERED HEAP. JUST AN EXEMPLAR FOR REPRESENTING A TREE USING AN ARRAY.

/*
                            (25)             <--- 3rd (Root/Top)
                           /    \
                          /      \
                         /        \
                        /          \
                      (20)         (22)      <--- 2nd
                     /    \        /  \
                    /      \      /    \
                  (17)     (19) (10)  (12)   <--- 1st
                  /  \     /  \
                 /    \   /    \
               (15)   (7)(9)  (18)           <--- 0th (Bottom)
*/

// the array representation will be:
// we move left -> right, through the hierarchies.

/*
 {25}:                                          (after traversing the 3rd level)
 {25, 20, 22}:                                  (after traversing the 2nd level)
 {25, 20, 22, 17, 19, 10, 12}:                  (after traversing the 1st level)
 {25, 20, 22, 17, 19, 10, 12, 15, 07, 09, 18}:  (after traversing the 0th level)
*/

// this design offers an inherent benefit that the parent node of a given node can easily be located by the
// offset (i - 1) / 2 where i is the offset of the chosen child node.

// E.g: what's the offset of node 19's parent?
// offset of node 19 is 4 (0 indexed array): i = 4
// (4 - 1) / 2 = 1 (integer division)
// node at offset 1 is 20, and it indeed is the parent of node 19.

// and the offset of a given node with offset i's left child is 2i + 1 and its right child is 2i + 2
// E.g. Take the node 20 for example, offset of node 20 is 1; i = 1
// left child = 2 * 1 + 1 = 3 and right child = 2 * 1 + 2 = 4
// 17 and 19 => correct!

// leftmost node must be the heaviest, and we do not care about the ordering of other nodes in the heap (array)

typedef struct _heap {     // heap
        uint32_t count;    // number of nodes.
        uint32_t capacity; // number of nodes the heap can hold before requiring a reallocation.
        bool (*predptr)(_In_ const void* const, _In_ const void* const);
        void** tree; // a heap allocated array containing pointers to heap allocated nodes.
                     // use malloc to allocate the tree and the nodes.
} heap_t;

static_assert(sizeof(heap_t) == 24);
static_assert(offsetof(heap_t, count) == 0);
static_assert(offsetof(heap_t, capacity) == 4);
static_assert(offsetof(heap_t, predptr) == 8);
static_assert(offsetof(heap_t, tree) == 16);

/*
// must return true whenever a swap is needed.
static inline bool __cdecl predicate(const void* const  child, const void* const  parent){
    retrun child->comparable > parent->comparable ? true : false;
}
*/

static inline bool __cdecl heap_init(
    _Inout_ heap_t* const restrict heap, _In_ bool (*const restrict predicate)(_In_ const void* const, _In_ const void* const)
) {
    heap->count    = 0;
    heap->capacity = DEFAULT_HEAP_CAPACITY; // this is the number of pointers, NOT BYTES
    heap->predptr  = predicate;
    // NOLINTNEXTLINE(bugprone-multi-level-implicit-pointer-conversion)
    heap->tree     = _CXX_COMPAT_REINTERPRET_CAST(void**, malloc(DEFAULT_HEAP_CAPACITY_BYTES));
    // (void**) malloc(DEFAULT_HEAP_CAPACITY_BYTES); // will allocate memory to store DEFAULT_HEAP_CAPACITY number of pointers in the tree.

    if (!heap->tree) {
        fwprintf_s(stderr, L"memory allocation error inside %s @LINE: %d\n", __FUNCTIONW__, __LINE__);
        return false;
    }

    memset(heap->tree, 0U, DEFAULT_HEAP_CAPACITY_BYTES); // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    return true;
}

static inline void __cdecl heap_clean(_Inout_ heap_t* const restrict heap) {
    for (unsigned i = 0; i < heap->count; ++i) {
        free(heap->tree[i]); // free the heap allocated nodes.
                             // heap->tree[i] = NULL;
    }
    free(heap->tree);                 // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    memset(heap, 0U, sizeof(heap_t)); // zero out the struct
}

static inline bool __cdecl heap_push(
    _Inout_ heap_t* const restrict heap,
    _In_ const void* const restrict pushed /* expects a heap allocated memory block to push into the heap */
) {
    void * _temp_node = NULL, **_temp_tree = NULL; // NOLINT(readability-isolate-declaration)
    size_t _childpos = 0, _parentpos = 0;          // NOLINT(readability-isolate-declaration)

    if (heap->count + 1 > heap->capacity) { // if the existing buffer doesn't have space for another pointer,
        dbgwprinf_s(L"reallocation inside %s\n", __FUNCTIONW__);

        // NOLINTNEXTLINE(bugprone-assignment-in-if-condition, bugprone-multi-level-implicit-pointer-conversion)
        if (!(_temp_tree = _CXX_COMPAT_REINTERPRET_CAST(void**, realloc(heap->tree, (heap->capacity + DEFAULT_HEAP_CAPACITY_BYTES))))) {
            // ask for an additional DEFAULT_HEAP_CAPACITY_BYTES bytes.
            // at this point, heap->tree is valid and points to the old buffer
            fwprintf_s(stderr, L"memory reallocation error inside %s @LINE: %d\n", __FUNCTIONW__, __LINE__);
            return false; // return false if the reallocation fails.
        }

        heap->tree      = _temp_tree;            // if reallocation was successful, reassign the new memory block to tree.
        heap->capacity += DEFAULT_HEAP_CAPACITY; // update the capacity
    }

    // consider our previous tree:
    // { 25, 20, 22, 17, 19, 10, 12, 15, 07, 09, 18 }
    // let's try and push a node with weight 24
    // after pushing we'd end up with an array like this, with the pushed node appended at the rightmost end.
    // { 25, 20, 22, 17, 19, 10, 12, 15, 07, 09, 18, 24 }
    // 24 is at offset 11, that makes it a child of node (i - 1) / 2 = 5, that is node 10.

    /* now the tree assumes a shape like this:

                               (25)               <--- 3rd (Root/Top)
                             /     \
                            /       \
                           /         \
                          /           \
                         /             \
                        /               \
                      (20)              (22)      <--- 2nd
                     /    \             /  \
                    /      \           /    \
                  (17)     (19)      (10)  (12)   <--- 1st
                  /  \     /  \      /
                 /    \   /    \    /
               (15)   (7)(9)  (18)(24)            <--- 0th (Bottom)
    */

    // since a child node cannot have a bigger weight than its parent node in a top-heavy heap, this needs reordering.
    // to make this push comply with the heap requisites, we'll swap child and parents until we get to an acceptable structure.

    // first swap is between 24 and its parent 10;
    /* now the tree assumes a shape like this:

                               (25)               <--- 3rd (Root/Top)
                             /     \
                            /       \
                           /         \
                          /           \
                         /             \
                        /               \
                      (20)              (22)      <--- 2nd
                     /    \             /  \
                    /      \           /    \
                  (17)     (19)      (24)  (12)   <--- 1st
                  /  \     /  \      /
                 /    \   /    \    /
               (15)   (7)(9)  (18)(10)            <--- 0th (Bottom)
    */

    // still we have 22 as parent of 24, swap these two again;
    /* now the tree assumes a shape like this:

                               (25)                <--- 3rd (Root/Top)
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
    // {25, 20, 24, 17, 19, 22, 12, 15, 7, 9, 18, 10}
    // perfecto :)))

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
    size_t _leftchildpos = 0, _rightchildpos = 0, _parentpos = 0, _pos = 0; // NOLINT(readability-isolate-declaration)
    void*  _temp = NULL;

    if (!heap->count) { // if the heap is empty,
        *popped = NULL;
        return false;
    }

    if (heap->count == 1) { // if the heap has only one node,
        *popped     = heap->tree[0];
        heap->count = 0;
        heap_clean(heap); // since heap->count = 0, the looped free()s inside heap_clean() won't be executed.
        return true;
    }

    // {25, 20, 24, 17, 19, 22, 12, 15, 7, 9, 18, 10}
    // give up the node at the top (root node)
    *popped                     = heap->tree[0];
    heap->tree[0]               = NULL;
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

    assert(!_parentpos);
    while (true) { // rearrange the tree
        // at the onset of iteration, parentpos = 0; currently points to the (new) root node.
        _leftchildpos  = lchild_position(_parentpos);
        _rightchildpos = rchild_position(_parentpos);

        // if we are not @ the end of the array and the weight of the left child is greater than that of the parent
        _pos = (_leftchildpos < (heap->count - 1)) && (*heap->predptr)(heap->tree[_leftchildpos], heap->tree[_parentpos]) ? _leftchildpos :
                                                                                                                            _parentpos;
        // choose to traverse down the left arm of the node, otherwise hold the caret at the parent.
        // in our example, this block will choose the left arm.

        // until we reach the rightmost end of the array
        // if the weight of the right child is greater than that of the parent OR the left child (CONTEXT DEPENDENT)
        if (_rightchildpos < heap->count - 1 && (*heap->predptr)(heap->tree[_rightchildpos], heap->tree[_pos])) {
            // choose to traverse down the right arm
            _pos = _rightchildpos;
            // in our example this conditional will choose the right arm, because 24 > 10.
        }

        // in our example, both conditionals at line 310 and 318 will evaluate to true.
        // since the expressions predicted on the conditional at line 318 will be evaluated finally,
        // it is the right child that will be swapped with the parent first.

        // if the root is heavier than both the left and right children, no need to rearrange the heap.
        if (_pos == _parentpos) break;

        _temp                     = heap->tree[_parentpos];
        heap->tree[_parentpos]    = heap->tree[_leftchildpos];
        heap->tree[_leftchildpos] = _temp;
        _temp                     = NULL;

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
        _parentpos                = _pos;
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
