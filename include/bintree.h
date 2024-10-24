#pragma once
#include <utilities.h>

// binary tree is a hierarchical arrangement of nodes, with each node having a parent node and (optionally - a pair of) child nodes
// each node usually has 3 members, one data members and two pointers, for left and right child nodes
// when a node does not have branches, we set the appropriate child pointer to NULL
// when a node has no children it is called a leaf node and these nodes usually occur at branch ends

// a binary tree node
typedef struct _btnode {
        struct btnode* left;  // pointer to left child node
        struct btnode* right; // pointer to right child node
        void*          data;
} btnode;

static_assert(sizeof(btnode) == 24);
static_assert(offsetof(btnode, left) == 0);
static_assert(offsetof(btnode, right) == 8);
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

typedef enum { LEFT = 0xFF << 0x02, RIGHT = 0xFF << 0x03 } TREENODE;

static inline bool __cdecl bntree_insert(
    _Inout_ bntree* const     tree,
    _Inout_opt_ btnode* const dest,  // the node of the binary tree where we want the `data` to be linked to
    _In_ const TREENODE       where, // which branch of the destination node the data needs to be connected to
    _In_ void* const          data   // the raw data that needs to be connected
) {
    assert(tree);
    assert(data);
    // dest can be NULL

    const btnode** target = NULL; // the node in the tree whose data needs to be updated
    btnode*        temp   = NULL;

    // if dest is NULL, then data can only be linked to the root node of the tree
    if (!dest) {
        // but we cannot change the root if the root node already has a non NULL data pointer as this would violate the tree structure
        if (!tree->node_count) {
            fwprintf_s(stderr, L"");
            return false;
        }
        target = &tree->root; // register the target node is the root node

    } else { // entertain the dest pointer

        switch (where) {
            case LEFT :
                {
                    if (dest->left) { // if the left pointer of the destination node is not NULL we cannot modify it
                        fwprintf(stderr, L"");
                        return false;
                    }
                    target = &dest->left; // register the target node is the left child of the destination node
                    break;
                }
            case RIGHT :
                {
                    if (dest->right) { // if the left pointer of the destination node is not NULL we cannot modify it
                        fwprintf(stderr, L"");
                        return false;
                    }
                    target = &dest->right; // register the target node is the right child of the destination node
                    break;
                }
            default : break;
        }
    }

    if (!(temp = malloc(sizeof(btnode)))) {
        fwprintf_s(stderr, L"");
        return false;
    }

    temp->data = data;               // hook in the data
    temp->left = temp->right = NULL; // make the left and right arms of the NULL
    *target                  = temp;
    return true;
}
