#pragma once
#include <utilities.h>

// binary tree is a hierarchical arrangement of nodes, with each node having a parent node and (optionally - a pair of) child nodes
// each node usually has 3 members, one data members and two pointers, for left and right child nodes
// when a node does not have branches, we set the appropriate child pointer to NULL
// when a node has no children it is called a leaf node and these nodes usually occur at branch ends

// a binary tree node
typedef struct _btnode {
        struct _btnode* left;  // pointer to left child node
        struct _btnode* right; // pointer to right child node
        void*           data;
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

typedef enum { LEFT = 0xFF << 0x02, RIGHT = 0xFF << 0x03 } NODE_ARM; // arms of a node

static inline bool __cdecl bntree_insert(
    _Inout_ bntree* const restrict tree,
    _Inout_opt_ btnode* const restrict parent, // the parent node in the binary tree where we want new node to be linked to
    _In_ const NODE_ARM where,                 // which arm of the parent node the data needs to be connected to
    _In_ void* const restrict data             // the raw data the new node needs to encapsulate
) {
    assert(tree);
    assert(data);
    // parent can be NULL

    btnode** restrict target = NULL; // the node in the tree whose data needs to be updated
    btnode* restrict temp    = NULL;

    // if parent is NULL, then data can only be linked to the root node of the tree
    if (!parent) {
        // but we cannot change the root if the root node already has a non NULL data pointer as this would violate the tree structure
        if (!tree->node_count) {
            fwprintf_s(stderr, L"");
            return false;
        }
        target = &tree->root; // register root node as the target node
    } else {                  // entertain the parent pointer
        switch (where) {
            case LEFT :
                {
                    if (parent->left) { // if the left pointer of the destination node is not NULL we cannot modify it
                        fwprintf(stderr, L"");
                        return false;
                    }
                    target = &parent->left; // register the target node is the left child of the destination node
                    break;
                }
            case RIGHT :
                {
                    if (parent->right) { // if the left pointer of the destination node is not NULL we cannot modify it
                        fwprintf(stderr, L"");
                        return false;
                    }
                    target = &parent->right; // register the target node is the right child of the destination node
                    break;
                }
            default : break;
        }
    }

    if (!(temp = (btnode*) malloc(sizeof(btnode)))) { // if the allocation fails
        fwprintf_s(stderr, L"");
        return false;
    }

    temp->data = data;               // wrap the data inside the new node
    temp->left = temp->right = NULL; // make the left and right arms of new node NULL
    *target                  = temp; // hook the new node into the binary tree
    tree->node_count++;              // account for the annexure
    return true;
}

static inline bool __cdecl bntree_remove(
    _Inout_ bntree* const restrict tree, _Inout_ btnode* const restrict parent, _In_ const NODE_ARM where
) {
    assert(tree);

    // handle the posibility that the tree is empty
    if (!tree->node_count) {
        fwprintf_s(stderr, L"");
        return false;
    }

    btnode** restrict target = NULL; // the target node where we want to remove the child

    if (!parent) // if parent is NULL, then our target becomes the root node
        target = &tree->root;
    else { // choose the intended parent's child node for removal
        switch (where) {
            case LEFT  : target = &parent->left; break;
            case RIGHT : target = &parent->right; break;
            default    : break;
        }
    }

    if (*target) { // if the chosen target node is not already NULL, remove its children before removing it
        bntree_remove(tree, *target, LEFT);
        bntree_remove(tree, *target, RIGHT);

        free(*target);
        *target = NULL;
        tree->node_count--; // account for the node removal
        // we don't have to account for the nodes that may have been removed by the recursive calls to bntree_remove() here
        // because those calls themselves would have registered those removals by decrementing the node count
    }

    return true;
}

static inline bool __cdecl bntree_merge(
    _Inout_ bntree* const restrict target,
    _In_ const bntree* const restrict left,
    _In_ const bntree* const restrict right,
    _In_ void* const restrict data
) { }