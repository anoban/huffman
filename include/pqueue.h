#pragma once
#include <utilities.h>

#define DEFAULT_PQUEUE_CAPACITY       1024LLU
#define DEFAULT_PQUEUE_CAPACITY_BYTES (DEFAULT_PQUEUE_CAPACITY * sizeof(uintptr_t))

// many problems demand retrieval of the smallest or largest element stored a collection which is capable of frequent insertions and deletions
// one way to meet this requirement is to keep the collection sorted all the time but sorting a collection after every insertion or deletion is expensive
// and becomes exponentially expensive as the collection grows

// fortunately this requirement does not demand that all the elements in a collection be sorted.
// it's enough for the collection to be aware of location of the smallest or the largest element, hence enabling rapid access to that element.
// heaps and priority queues are adept for this purpose
// a priority queue can be implemented easily by encapsulating a heap

// heaps are organized trees (usually a binary tree) that allow easy access to the largest / smallest element.
// the cost of creating and maintaining a tree is negligible compared to the performance overhead incurred by maintaining the container sorted all the time

// in a heap, each child node has a weight smaller than its parent's, ergo it is the root node that has the largest weight in a heap
// binary trees like these are top heavy, as we climb up the strata of nodes, nodes will become heavier
// trees can also be engineered to hold the heaviest nodes at the bottom and lightest nodes at the top, where the root node will be the lightest
// in that case, parent nodes will have a smaller weight compared to their children, hence making the tree bottom heavy

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

// this design offers an inherent benefit that the parent node of a given node can easily be located by the equation
// = (i - 1) / 2 where i is the offset of the chosen child node.

// e.g: what's the offset of node 19's parent?
// offset of node 19 is 4 (0 indexed array): i = 4
// (4 - 1) / 2 = 1 (integer division)
// node at offset 1 is 20, and it indeed is the parent of node 19.

// and the offset of a given node's (offset i) left child is (2i + 1) and right child is (2i + 2)
// e.g. take the node 20 for example, offset of node 20 is 1; i = 1
// left child = 2 * 1 + 1 = 3 and right child = 2 * 1 + 2 = 4
// 17 and 19 => correct!

// leftmost node must be the heaviest, and we do not care about the ordering of other nodes in heaps & priority queues

typedef struct _pqueue {   // priority queue
        uint32_t count;    // number of nodes.
        uint32_t capacity; // number of nodes the prqueue can hold before requiring a reallocation.
        bool (*predptr)(_In_ const void* const, _In_ const void* const);
        void** tree; // a heap allocated array containing pointers to heap allocated nodes. use malloc to allocate the tree and the nodes.
} pqueue;

static_assert(sizeof(pqueue) == 24);
static_assert(offsetof(pqueue, count) == 0);
static_assert(offsetof(pqueue, capacity) == 4);
static_assert(offsetof(pqueue, predptr) == 8);
static_assert(offsetof(pqueue, tree) == 16);

/*
// must return true whenever a swap is needed.
static inline bool __cdecl predicate(const void* const  child, const void* const  parent){
    retrun child->comparable > parent->comparable;
}
*/

static inline bool __cdecl pqueue_init(
    _Inout_ pqueue* const prqueue, _In_ bool (*const predicate)(_In_ const void* const child, _In_ const void* const parent)
) {
    assert(prqueue);
    assert(predicate);

    // NOLINTNEXTLINE(bugprone-multi-level-implicit-pointer-conversion)
    prqueue->tree = (void**) malloc(DEFAULT_PQUEUE_CAPACITY_BYTES);
    // will allocate memory to store DEFAULT_PQUEUE_CAPACITY number of pointers in the tree.

    if (!prqueue->tree) {
        fputws(L"Error:: malloc failed inside " __FUNCTIONW__ "\n", stderr);
        return false;
    }

    prqueue->count    = 0;
    prqueue->capacity = DEFAULT_PQUEUE_CAPACITY; // this is the number of pointers, NOT BYTES
    prqueue->predptr  = predicate;
    memset(prqueue->tree, 0U, DEFAULT_PQUEUE_CAPACITY_BYTES); // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    return true;
}

static inline void __cdecl pqueue_clean(_Inout_ pqueue* const prqueue) {
    for (size_t i = 0; i < prqueue->count; ++i) free(prqueue->tree[i]); // free the heap allocated nodes.
    free(prqueue->tree);                                                // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    memset(prqueue, 0U, sizeof(pqueue));                                // zero out the struct
}

// enqueue
static inline bool __cdecl pqueue_push(
    _Inout_ pqueue* const prqueue, _In_ void* const data /* expects a heap allocated memory block to push into the prqueue */
) {
    assert(prqueue);
    assert(data);

    void * _temp_node = NULL, **_temp_tree = NULL; // NOLINT(readability-isolate-declaration)
    size_t _childpos = 0, _parentpos = 0;          // NOLINT(readability-isolate-declaration)

    // if the current buffer doesn't have space for another pointer, ask for an additional DEFAULT_PQUEUE_CAPACITY_BYTES bytes.
    if (prqueue->count + 1 > prqueue->capacity) {
        // NOLINTBEGIN(bugprone-assignment-in-if-condition, bugprone-multi-level-implicit-pointer-conversion)
        if (!(_temp_tree = (void**) realloc(
                  prqueue->tree,
                  (prqueue->capacity * sizeof(uintptr_t)) /* size of the existing buffer in bytes */ + DEFAULT_PQUEUE_CAPACITY_BYTES
              ))) {
            // NOLINTEND(bugprone-assignment-in-if-condition, bugprone-multi-level-implicit-pointer-conversion)
            fputws(L"Error:: realloc failed inside " __FUNCTIONW__ "\n", stderr);
            // at this point, prqueue->tree is still valid and points to the old buffer
            return false; // return false if reallocation fails.
        }

        prqueue->tree      = _temp_tree;              // reallocation succeeded, reassign the new memory block to tree.
        prqueue->capacity += DEFAULT_PQUEUE_CAPACITY; // update the capacity
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

    prqueue->count++;                                               // increment the node count
    prqueue->tree[prqueue->count - 1] = data;                       // hook in the new node at the end
    _childpos                         = prqueue->count - 1;         // offset of the newly inserted node.
    _parentpos                        = parent_position(_childpos); // offset of the new node's parent.

    // start the reordering with the newly pushed node
    while ((_childpos > 0) // as long as we haven't reached the root node at offset 0, (i.e the new node isn't the only node in the heap)
           &&              // and the child node is heavier than the parent node
           (*prqueue->predptr)(prqueue->tree[_childpos], prqueue->tree[_parentpos])) { // keep rearranging the priority queue

        _temp_node                = prqueue->tree[_childpos];  // child node to be pushed up
        prqueue->tree[_childpos]  = prqueue->tree[_parentpos]; // push the parent node down
        prqueue->tree[_parentpos] = _temp_node;                // push the child node up

        // prepare for the next swap in anticipation that the reordering we just did also caused a disorder in the heap structure
        _childpos                 = _parentpos;                 // after the swap, parent's offset becomes child's offset
        _parentpos                = parent_position(_childpos); // find the parent of the new child
    }

    return true;
}

// dequeue
static inline bool __cdecl pqueue_pop(_Inout_ pqueue* const prqueue, _Inout_ void** const popped /* popped out pointer */) {
    assert(prqueue);
    assert(popped);

    if (!prqueue->count) { // if the prqueue is already empty
        *popped = NULL;
        return false;
    }

    if (prqueue->count == 1) { // if the prqueue has only one node
        *popped        = prqueue->tree[0];
        prqueue->count = 0;
        pqueue_clean(prqueue); // since prqueue->count is 0, the looped free()s inside pqueue_clean() won't be executed.
        return true;
    }

    size_t _leftchildpos = 0, _rightchildpos = 0, _parentpos = 0, _pos = 0; // NOLINT(readability-isolate-declaration)
    void*  _temp     = NULL;

    // {25, 20, 24, 17, 19, 22, 12, 15, 7, 9, 18, 10}
    *popped          = prqueue->tree[0]; // give up the node at the top (root node)
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

    prqueue->tree[0] = prqueue->tree[prqueue->count - 1]; // move the last node in the array to the root's position, offset 0.
    prqueue->tree[prqueue->count - 1] = NULL;
    prqueue->count--; // register the loss of a node

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

    while (true) { // we start the loop with the new root node
        _leftchildpos  = lchild_position(_parentpos);
        _rightchildpos = rchild_position(_parentpos);

        // unless we are at the right end of the array and the left child is heavier than the parent
        _pos = (_leftchildpos <= (prqueue->count - 1)) && (*prqueue->predptr)(prqueue->tree[_leftchildpos], prqueue->tree[_parentpos]) ?
                   _leftchildpos :
                   _parentpos;
        // choose to traverse down the left arm of the node, otherwise hold the caret at the parent.
        // in our example, this block will choose the left arm.

        // unless we are at the right end of the array and the right child is heavier than the parent OR its sibling (the left child)
        // (CONTEXT DEPENDENT) - has pos been updated to the left child's offset before,
        if ((_rightchildpos <= (prqueue->count - 1)) && (*prqueue->predptr)(prqueue->tree[_rightchildpos], prqueue->tree[_pos]))
            _pos = _rightchildpos;
        // choose to traverse down the right arm
        // in our example this conditional will choose the right arm, because 24 > 10.

        // in our example, both conditionals at line 311 and 319 will evaluate to true.
        // since the expressions predicted on the conditional at line 319 will be evaluated finally, it is the right child that will ultimately be swapped with the parent

        if (_pos == _parentpos) break; // if the root is heavier than both its left and right children, no need to rearrange the heap.

        _temp                     = prqueue->tree[_parentpos];
        prqueue->tree[_parentpos] = prqueue->tree[_pos];
        prqueue->tree[_pos]       = _temp;

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

        _parentpos                = _pos; // traverse down. i.e the current right child becomes the parent node for the next iteration.
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

static inline void* __stdcall pqueue_peek(_In_ const pqueue* const prqueue) { return prqueue->tree ? prqueue->tree[0] : NULL; }
