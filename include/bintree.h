#pragma once
#include <utilities.h>

// binary tree is a hierarchical arrangement of nodes, with each node having a parent node and (optionally - a pair of) child nodes
// each node usually has 3 members, one data members and two pointers, for left and right child nodes
// when a node does not have branches, we set the appropriate child pointer to NULL
// when a node has no children it is called a leaf node and these nodes usually occur at branch ends

// a binary tree node
typedef struct _btnode {
        struct btnode* child_left;  // pointer to left child node
        struct btnode* child_right; // pointer to right child node
        void*          data;
} btnode;

static_assert(sizeof(btnode) == 24);
static_assert(offsetof(btnode, child_left) == 0);
static_assert(offsetof(btnode, child_right) == 8);
static_assert(offsetof(btnode, data) == 16);

// a binary tree
typedef struct _bintree {
        size_t  node_count; // number of nodes in the binary tree
        btnode* root;       // root node of the binary tree
        // a function pointer that can compare binary tree node's data types
        bool (*predptr)(_In_ const void* const, _In_ const void* const);
} bntree;

static_assert(sizeof(bntree) == 24);
static_assert(offsetof(bntree, node_count) == 0);
static_assert(offsetof(bntree, root) == 8);
static_assert(offsetof(bntree, predptr) == 16);

static inline bool InsertLeft(
    _Inout_ bntree* const tree,
    _Inout_ btnode* const dest, // the node of the binary tree where we want the `data` to be linked to
    _In_ void* const      data  // the raw data that needs to be connected
) {
    assert(tree);
    assert(data);

    malloc(sizeof(btnode));
    //
}
