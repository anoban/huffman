#pragma once
#include <utilities.h>

// binary tree is a hierarchical arrangement of nodes, with each node having a parent node and (optionally - a pair of) child nodes
// each node usually has 3 members, one data members and two pointers, for left and right child nodes
// when a node does not have branches, we set the appropriate child pointer to NULL
// when a node has no children it is called a leaf node and these nodes usually occur at branch ends

//------------------------------------------------------------------------------------------------------//
// THIS BINARY TREE IMPLEMENTATION DOES NOT DO ANY SORTING OR TREE STRUCTURE INTEGRITY ENFORCEMENTS     //
// ON ITS OWN. HENCE IT IS THE USER'S RESPONSIBILITY TO MAINTAIN THE TREE'S STRUCTURE WHEN USING IT     //
//------------------------------------------------------------------------------------------------------//

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
} bntree;

static_assert(sizeof(bntree) == 16);
static_assert(offsetof(bntree, node_count) == 0);
static_assert(offsetof(bntree, root) == 8);

typedef enum _child_kind { ROOT = 0xFF << 0x01, LEFT = 0xFF << 0x02, RIGHT = 0xFF << 0x03 } child_kind; // arms of a node

// create a node with the data provided and insert it into the binary tree as the child of the specified parent node
static inline bool __cdecl bntree_insert(
    _Inout_ bntree* const restrict tree,
    _Inout_opt_ btnode* const restrict parent, // the parent node in the binary tree where we want new node to be linked to
    _In_ const child_kind which,               // which arm of the parent node the data needs to be connected to
    // when parent is NULL, argument which will not be used
    _In_ void* const restrict data // the raw data the new node needs to encapsulate
) {
    assert(tree);
    assert(data);
    // parent can be NULL

    btnode** restrict target = NULL; // the node in the tree whose data needs to be updated
    btnode* restrict temp    = NULL;

    if (!parent) { // if parent is NULL, then data can only be linked to the root node of the tree
        // but we cannot change the root if the root node already has children this would violate the tree structure
        if (tree->node_count) {
            fputws(L"Error:: " __FUNCTIONW__ " does not allow insertions into a non NULL root nodes!\n", stderr);
            return false;
        }
        target = &tree->root; // register root node as the target node

    } else { // entertain the parent pointer
        switch (which) {
            case LEFT :
                {
                    if (parent->left) { // if the left pointer of the destination node is not NULL we cannot update it
                        fputws(L"Error:: " __FUNCTIONW__ " does not allow insertions into a non NULL child nodes!\n", stderr);
                        return false;
                    }
                    target = &parent->left; // register the target node is the left child of the destination node
                    break;
                }
            case RIGHT :
                {
                    if (parent->right) { // if the left pointer of the destination node is not NULL we cannot update it
                        fputws(L"Error:: " __FUNCTIONW__ " does not allow insertions into a non NULL child nodes!\n", stderr);
                        return false;
                    }
                    target = &parent->right; // register the target node is the right child of the destination node
                    break;
                }
            default : break;
        }
    }

    if (!(temp = (btnode*) malloc(sizeof(btnode)))) { // NOLINT(bugprone-assignment-in-if-condition) if the allocation fails
        fputws(L"Error:: malloc failed inside " __FUNCTIONW__ "\n", stderr);
        return false;
    }

    temp->data = data;               // wrap the data inside the new node
    temp->left = temp->right = NULL; // make the left and right arms of new node NULL
    *target                  = temp; // hook the new node into the binary tree
    tree->node_count++;              // account for the annexure

    return true;
}

// remove the child of the specified parent node from the given binary tree
static inline bool __cdecl bntree_remove( // NOLINT(misc-no-recursion)
    _Inout_ bntree* const restrict tree,
    _Inout_opt_ btnode* const restrict parent,
    _In_ const child_kind which
) {
    assert(tree);
    // parent can be NULL

    if (!tree->node_count) { // handle the possibility that the tree is empty
        fputws(L"Error:: " __FUNCTIONW__ " cannot remove nodes from an empty binary tree\n", stderr);
        return false;
    }

    btnode** restrict target = NULL; // the target node where we want to remove the child

    if (!parent) // if parent is NULL, then our target becomes the root node
        target = &tree->root;
    else { // choose the intended parent's child node for removal
        switch (which) {
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

// merge two binary trees into one
static inline bntree __cdecl bntree_merge(
    _In_ bntree* const restrict left,  // to be merged
    _In_ bntree* const restrict right, // to be merged
    _In_ void* const restrict data     // data for the new root node
) {
    assert(left);
    assert(right);
    assert(data);

    bntree merged = { .node_count = 0, .root = NULL };
    if (!bntree_insert(
            &merged,
            NULL,
            ROOT, // this is just a placeholder here because since the parent is NULL our target becomes the root node, the control flow won't even reach the switch block
            data
        )) { // if the insertion failed
        fputws(L"Error:: ", stderr);
        return merged;
    }

    // link the subtrees appropriately to the merged tree's root node
    merged.root->left  = left->root;
    merged.root->right = right->root;
    merged.node_count +=
        left->node_count + right->node_count; // update the merged tree's node count to account for the nodes in the subtrees

    // since the merged tree has taken ownership of the nodes in the subtrees, they no longer belong to them
    // make them inaccessible to those trees
    left->root = right->root = NULL;
    left->node_count = right->node_count = 0;

    return merged;
}
