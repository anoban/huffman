#pragma once
#ifndef __HEAP_H__
    #define __HEAP_H__
    #include <memory.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include <stdlib.h>

// Code improvised from Mastering Algorithms with C (1999) Kyle Loudon

    #define HPCAP 1024Ui64

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

// Heaps are typically left balanced. When new nodes are annexed to the tree at a given level, the tree grows from left to right.

// An efficient way to implement left-balanced binary trees is to store the nodes in a contiguous array.
// Nodes are arranged in this array in the order we'd encounter them in a level traversal.

// Say that we have 11 leaf nodes with weights 15, 7, 9, 18, 10, 12, 17, 19, 20, 22 ans 25 to begin with.
// NOTE: Parent nodes here, aren't created from the children!
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

// The array representation will be:
// We move left -> right, through the hierarchies.

// {25}:                                         after traversing the 3rd level
// {25, 20, 22}:                                 after traversing the 2nd level
// {25, 20, 22, 17, 19, 10, 12}:                 after traversing the 1st level
// {25, 20, 22, 17, 19, 10, 12, 15, 07, 09, 18}: after traversing the 0th level

// This design offers an inherent benefit where the parent node of a given node can easily be located by the
// offset (i - 1) / 2 where i is the offset of the select node.

// E.g: What's the offset of node 19's parent?
// Offset of node 19 is 4 (0 indexed array): i = 4
// (4 - 1) integer division 2 = 1
// node at offset 1 is 20, and it indeed is the parent of node 19.

// And the offset of a given node with offset i's left child is 2i + 1
// and the offset of the right child is 2i + 2
// E.g. Take the node 20 for example, offset of node 20 is 1; i = 1
// left child = 2 * 1 + 1 = 3 and right child = 2 * 1 + 2 = 4
// 17 and 19 => correct!

// Leftmost node must be the heaviest, and we do not care about the ordering of other nodes in the heap (array)

static inline size_t __stdcall parentPos(_In_ const size_t child_pos) { return (child_pos - 1) / 2; } // truncating division.
static inline size_t __stdcall leftChildPos(_In_ const size_t parent_pos) { return (parent_pos * 2) + 1; }
static inline size_t __stdcall rightChildPos(_In_ const size_t parent_pos) { return (parent_pos * 2) + 2; }

typedef struct _heap {
        uint64_t count;                                                                               // number of nodes.
        uint64_t capacity; // number of nodes the heap can hold before requiring a reallocation.
        bool     (*fnptr_pred)(_In_reads_(1) const void* const restrict child, _In_reads_(1) const void* const restrict parent);
        void     (*fnptr_clean)(_In_reads_(1) const void* const restrict memblock);
        void**   tree;     // a heap allocated array containing pointers to heap allocated nodes.
                           // use malloc to allocate the tree and the nodes.
} heap_t;

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

static inline bool heapInit(
    _Inout_ heap_t* const restrict heap,
    _In_ const bool (*predicate)(_In_reads_(1) const void* const restrict child, _In_reads_(1) const void* const restrict parent),
    _In_ const void (*clean)(_In_reads_(1) const void* const restrict memblock)
) {
    heap->count       = 0;
    heap->capacity    = HPCAP;
    heap->fnptr_pred  = predicate;
    heap->fnptr_clean = clean;
    // will initially allocate memory to store 1024 pointers in the tree.
    if (!(heap->tree = malloc(HPCAP * sizeof(uintptr_t)))) return false;
    memset(heap->tree, 0U, HPCAP * sizeof(uintptr_t));
    return true;
}

static inline void heapClean(_Inout_ heap_t* const restrict heap) {
    for (size_t i = 0; i < heap->count; ++i) free(heap->tree[i]); // free the heap allocated nodes.
    free(heap->tree);                                             // free the array containing pointers to heap allocated nodes.
    // two levels of heap allocation happens here!
    memset(heap, 0U, sizeof(heap_t));
    return;
}

static inline bool heapPush(
    _Inout_ heap_t* const restrict heap, _In_ const void* const restrict data /* expects a heap allocated memory block (node) to push in */
) {
    void*  tmp      = NULL;
    size_t childpos = 0, parentpos = 0;

    // if the buffer doesn't have space for another pointer,
    if (heap->count + 1 > heap->capacity) {
        // ask for an additional 1024 * sizeof(uintptr_t) bytes. return false if the reallocation has failed.
        // genuinely fancying an arena allocator here!
        if (!(tmp = realloc(heap->tree, (heap->count + HPCAP) * sizeof(uintptr_t)))) return false;
        heap->tree = tmp; // if reallocation was successful, reassign the new memory block to tree.
    }

    heap->capacity          = heap->count + HPCAP;

    // Consider our previous tree:
    // { 25, 20, 22, 17, 19, 10, 12, 15, 07, 09, 18 }
    // let's try and push a node with weight 24
    // after pushing we'd end up with an array like this, with the pushed node appended at the rightmost end.
    // { 25, 20, 22, 17, 19, 10, 12, 15, 07, 09, 18, 24 }
    // 24 is at offset 11, that makes it a child of node (i - 1) / 2 = 5, that is node 10.

    /* Now the tree assumes a shape like this:

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

    // Since a child node cannot have a higher weight than a parent node in a top-heavy heap, this needs doctoring.
    // To make this push comply to the heap data structure, we'll swap child and parents until we get to an acceptable structure.

    // First swap is between 24 and its parent 10;
    /* Now the tree assumes a shape like this:

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
    // Still we have 22 as parent of 24, swap these two again;
    /* Now the tree assumes a shape like this:
    *
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
    // Perfecto :)))

    heap->tree[heap->count] = data;                // insert the new node, it's not count + 1! offsets start at 0.
    childpos                = heap->count;         // offset of the newly inserted node.
    parentpos               = parentPos(childpos); // offset of the new node's parent.

    while ((childpos > 0) /* as long as we haven't reached the root node at offset 0 */ &&
           heap->fnptr_pred(heap->tree[childpos], heap->tree[parentpos])) {
        // and the weight of the child is greater than that of the parent. (two conditions needed to perform the swap)

        tmp                   = heap->tree[childpos];
        heap->tree[parentpos] = heap->tree[childpos];
        heap->tree[childpos]  = tmp;
        tmp                   = NULL;

        // prepare for the next swap in anticipation that the new positioning of the inserted node also needs a swap.
        childpos              = parentpos;           // previous parent's offset now becomes the child's offset.
        parentpos             = parentPos(childpos); // find the parent of the current child's offset.
    }

    heap->count++;                                   // increment the node count.
    return true;
}

static inline bool heapPop(_Inout_ heap_t* const restrict heap, _Inout_ void** restrict data /* popped out */) {
    size_t leftchildpos = 0, rightchildpos = 0, parentpos = 0, pos = 0;
    void*  tmp = NULL;

    // if the heap is empty,
    if (!heap->count) return false;

    // If the heap has only one node,
    if (heap->count == 1) {
        *data = heap->tree[0];
        heap->count--;
        heapClean(heap); // since heap->count = 0, the looped free()s inside heapClean() won't be executed.
        return true;
    }

    // {25, 20, 24, 17, 19, 22, 12, 15, 7, 9, 18, 10} ----------\
                                                             // |
    // give up the node at the top (root node)               // |
    *data                       = heap->tree[0]; // |
    heap->tree[0]               = NULL;          // |
                                                 // |
    // {NULL, 20, 24, 17, 19, 22, 12, 15, 7, 9, 18, 10} <-------/

    /* Now the tree looks like this:

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

    // Move the last node in the array to the root's position, offset 0.
    heap->tree[0]               = heap->tree[heap->count - 1];
    heap->tree[heap->count - 1] = NULL;
    heap->count--;

    /* Now the tree looks like this:

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
    // rearrange the tree

    while (true) {
        // At the onset of iteration, parentpos = 0; currently points to the (new) root node.
        leftchildpos  = leftChildPos(parentpos);
        rightchildpos = rightChildPos(parentpos);

        if (leftchildpos < heap->count /* until we reach the rightmost end */ &&
            heap->fnptr_pred(
                heap->tree[leftchildpos], heap->tree[parentpos]
            )) { // if the weight of the left child is greater than that of the parent,
            // choose to traverse down the left arm.
            pos = leftchildpos;
        } else {
            pos = parentpos; // otherwise hold the caret at the parent.
        }
        // In our example, this conditional will choose the left arm.

        if (rightchildpos < heap->count /* until we reach the rightmost end */ &&
            heap->fnptr_pred(
                heap->tree[rightchildpos], heap->tree[pos]
            )) { // if the weight of the right child is greater than that of the parent OR the left child (CONTEXT DEPENDENT)
            // choose to traverse down the right arm
            pos = rightchildpos;
            // In our example this conditional will choose the right arm, because 24 > 10.
        }

        // In our example, both conditionals at line 310 and 318 will evaluate to true.
        // Since the expressions predicted on the conditional at line 318 will be evaluated finally,
        // It is the right child that will be swapped with the parent first.

        // if the root is heavier than both the left and right children, no need to rearrange the heap.
        if (pos == parentpos) break;

        tmp                      = heap->tree[parentpos];
        heap->tree[parentpos]    = heap->tree[leftchildpos];
        heap->tree[leftchildpos] = tmp;
        tmp                      = NULL;

        /* Now the tree looks like this:

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
        parentpos                = pos;
    }

    /* At the start of second iteration, the tree looks like this:

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
    // Again the first conditional will pick the left arm for swapping as 22 > 10.
    // Second conditional won't execute as 22 < 12, finally the left arm will be selected for swapping.

    /* At the end of second iteration, the tree looks like this:

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
    // No rearrangements are needed anymore.

    return true;
}

#endif // !__HEAP_H__