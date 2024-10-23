#pragma once
#include <utilities.h>

#define DEFAULT_PQUEUE_CAPACITY       1024LLU
#define DEFAULT_PQUEUE_CAPACITY_BYTES (DEFAULT_PQUEUE_CAPACITY * sizeof(uintptr_t))

// many problems demand determination of the smallest or largest element from a collection that's capable of frequent insertions and deletions
// one way to meet this need is to keep the collection sorted all the time but sorting a collection at every insertion or deletion is expensive

// fortunately this requirement does not insinuate that all the elements in a collection remain sorted.
// it's enough for the collection to just know the location of the smallest or the largest element, hence enabling rapid access to that element.
// heaps and priority queue are adept for this purpose
// a priority queue can be implemented easily by encapsulating a heap

// heaps are organized trees (usually a binary tree) that allow easy access to the largest / smallest element.
// the cost of creating and maintaining a tree is negligible compared to the performance overhead incurred by maintaining the container sorted all the time

// in a heap, each child node has a weight smaller than its parent's, ergo it is the root node that has the largest weight in a heap
// binary trees like these are top heavy, as we climb up the hierarchy of nodes, nodes will become more and more heavy (will have a larger weight)

// trees can also be engineered to hold the heaviest nodes at the bottom and lightest nodes at the top, whereby the root node will be the one with the smallest weight
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

// leftmost node must be the heaviest, and we do not care about the ordering of other nodes in heaps & priority queues

typedef struct _pqueue {   // priority queue
        uint32_t count;    // number of nodes.
        uint32_t capacity; // number of nodes the heap can hold before requiring a reallocation.
        bool (*predptr)(_In_ const void* const, _In_ const void* const);
        void** tree; // a heap allocated array containing pointers to heap allocated nodes. use malloc to allocate the tree and the nodes.
} PQueue;

static_assert(sizeof(PQueue) == 24);
static_assert(offsetof(PQueue, count) == 0);
static_assert(offsetof(PQueue, capacity) == 4);
static_assert(offsetof(PQueue, predptr) == 8);
static_assert(offsetof(PQueue, tree) == 16);

/*
// must return true whenever a swap is needed.
static inline bool __cdecl predicate(const void* const  child, const void* const  parent){
    retrun child->comparable > parent->comparable ? true : false;
}
*/

// using pascal case for queues
static inline bool __cdecl PQueueInit(
    _Inout_ PQueue* const pqueue, _In_ bool (*const predicate)(_In_ const void* const child, _In_ const void* const parent)
) {
    assert(pqueue);
    assert(predicate);

    // NOLINTNEXTLINE(bugprone-multi-level-implicit-pointer-conversion)
    pqueue->tree = _CXX_COMPAT_REINTERPRET_CAST(void**, malloc(DEFAULT_PQUEUE_CAPACITY_BYTES));
    // will allocate memory to store DEFAULT_PQUEUE_CAPACITY number of pointers in the tree.

    if (!pqueue->tree) {
        fwprintf_s(stderr, L"malloc failed inside %s @LINE: %d\n", __FUNCTIONW__, __LINE__);
        return false;
    }

    pqueue->count    = 0;
    pqueue->capacity = DEFAULT_PQUEUE_CAPACITY; // this is the number of pointers, NOT BYTES
    pqueue->predptr  = predicate;
    memset(pqueue->tree, 0U, DEFAULT_PQUEUE_CAPACITY_BYTES); // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    return true;
}

static inline void __cdecl PQueueClean(_Inout_ PQueue* const pqueue) {
    for (size_t i = 0; i < pqueue->count; ++i) free(pqueue->tree[i]); // free the heap allocated nodes.
    free(pqueue->tree);                                               // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    memset(pqueue, 0U, sizeof(PQueue));                               // zero out the struct
}

// enqueue
static inline bool __cdecl PQueuePush(
    _Inout_ PQueue* const pqueue, _In_ void* const data /* expects a heap allocated memory block to push into the heap */
) {
    assert(pqueue);
    assert(data);

    void * _temp_node = NULL, **_temp_tree = NULL; // NOLINT(readability-isolate-declaration)
    size_t _childpos = 0, _parentpos = 0;          // NOLINT(readability-isolate-declaration)

    if (pqueue->count + 1 > pqueue->capacity) {
        // ask for an additional DEFAULT_PQUEUE_CAPACITY_BYTES bytes.

        // NOLINTNEXTLINE(bugprone-assignment-in-if-condition, bugprone-multi-level-implicit-pointer-conversion)
        if (!(_temp_tree = _CXX_COMPAT_REINTERPRET_CAST(
                  void**,
                  realloc(
                      pqueue->tree,
                      (pqueue->capacity * sizeof(uintptr_t)) /* size of the existing buffer in bytes */ + DEFAULT_PQUEUE_CAPACITY_BYTES
                  )
              ))) {
            fwprintf_s(stderr, L"realloc failed inside %s @LINE: %d\n", __FUNCTIONW__, __LINE__);
            // at this point, heap->tree is still valid and points to the old buffer
            return false; // return false if reallocation fails.
        }
        pqueue->tree      = _temp_tree;              // if reallocation was successful, reassign the new memory block to tree.
        pqueue->capacity += DEFAULT_PQUEUE_CAPACITY; // update the capacity
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
static inline bool __cdecl PQueuePop(_Inout_ PQueue* const pqueue, _Inout_ void** const data) {
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

static inline void* __stdcall PQueuePeek(_In_ const PQueue* const pqueue) { return pqueue->tree ? pqueue->tree[0] : NULL; }
