#pragma once
#include <utilities.h>

// binary tree is a hierarchical arrangement of nodes, with each node having a parent node and (optionally - a pair of) child nodes
// each node usually has 3 members, one data members and two pointers, for left and right child nodes
// when a node does not have branches, we set the appropriate child pointer to NULL
// when a node has no children it is called a leaf node and these nodes usually occur at the branch ends

// a binary tree node
typedef struct _btnode {
        struct BtNode* child_left;  // pointer to left child node
        struct BtNode* child_right; // pointer to right child node
        void*          data;
} BtNode;

static_assert(sizeof(BtNode) == 24);
static_assert(offsetof(BtNode, child_left) == 0);
static_assert(offsetof(BtNode, child_right) == 8);
static_assert(offsetof(BtNode, data) == 16);

// a binary tree
typedef struct _bintree {
        size_t  node_count; // number of nodes in the binary tree
        BtNode* root;       // root node of the binary tree
        // a function pointer that can compare binary tree node's data types
        bool (*predptr)(_In_ const void* const, _In_ const void* const);
} BnTree;

static_assert(sizeof(BnTree) == 24);
static_assert(offsetof(BnTree, node_count) == 0);
static_assert(offsetof(BnTree, root) == 8);
static_assert(offsetof(BnTree, predptr) == 16);

static inline bool InsertLeft(
    _Inout_ BnTree* const bintree,
    _Inout_ BtNode* const dest, // the node of the binary tree where we want the `data` to be linked to
    _In_ void* const      data  // the raw data that needs to be connected
) {
    assert(bintree);
    assert(data);

    malloc(sizeof(BtNode));
    //
}