#pragma once
#include <fileio.h>
#include <stdint.h>
#include <utilities.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// the problem with the data structures in the other headers inside ./include/ is that they liberally rely on type erasure for the sake of supporting arbitraty user defined types
// we lose a lot of type safety and performance because this requires storing type erased heap allocated pointers instead of directly storing the values
// as this would make these data structures tightly coupled to a single type, hence losing their "library" status

// since this is not important to us and we need maximum performance, we'll make variants tailor made for huffman algorithms
// embedding associated user defined types directly inside the data structures, leveraging the tight coupling to eliminate
// type erasure, gratuitous heap allocations and maximize stack usage
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

#pragma region __STRUCT_DEFINITIONS

typedef union _symbol {
        unsigned char      byte; // only applicable to leaf nodes
        // could use a smaller type for marker but the member freq inside hnode_t will warrant padding, so why not use that here
        unsigned long long marker; // a value (UINT32_MAX) registering a non-leaf node
} symbol_t;

static_assert(sizeof(symbol_t) == 8);
static_assert(!offsetof(symbol_t, byte));
static_assert(!offsetof(symbol_t, marker));

typedef struct _hnode {            // represents a Huffman node.
        symbol_t           symbol; // byte for a leaf node, UINT32_MAX for internodes
        unsigned long long freq;
} hnode_t;

static_assert(sizeof(hnode_t) == 16);
static_assert(!offsetof(hnode_t, symbol));
static_assert(offsetof(hnode_t, freq) == 8);

typedef struct _hcode { // represents a Huffman code
        bool           is_used;
        unsigned char  length;
        unsigned short code;
} hcode_t; // no padding yeehaw :)

static_assert(sizeof(hcode_t) == 4);
static_assert(!offsetof(hcode_t, is_used));
static_assert(offsetof(hcode_t, length) == 1);
static_assert(offsetof(hcode_t, code) == 2);

typedef struct _btnode {
        struct _btnode* left;
        struct _btnode* right;
        hnode_t         data;
} btnode_t;

static_assert(sizeof(btnode_t) == 16 + sizeof(hnode_t));
static_assert(offsetof(btnode_t, left) == 0);
static_assert(offsetof(btnode_t, right) == 8);
static_assert(offsetof(btnode_t, data) == 16);

typedef struct _bintree {
        unsigned long long node_count;
        btnode_t*          root;
} bntree_t;

static_assert(sizeof(bntree_t) == 16);
static_assert(offsetof(bntree_t, node_count) == 0);
static_assert(offsetof(bntree_t, root) == 8);

typedef struct _pqueue {
        unsigned  count;
        unsigned  capacity;
        bntree_t* tree;
} pqueue_t;

static_assert(sizeof(pqueue_t) == 16);
static_assert(offsetof(pqueue_t, count) == 0);
static_assert(offsetof(pqueue_t, capacity) == 4);
static_assert(offsetof(pqueue_t, tree) == 8);

#pragma endregion

#pragma region __TAILOR_MADE_PQUEUE

#define _PQUEUE_FIXEDCAPACITY       (2LLU << 10)
#define _PQUEUE_FIXEDCAPACITY_BYTES (_PQUEUE_FIXEDCAPACITY * sizeof(hnode_t))

static bntree_t __PQUEUE_GLOBAL_BUFFER[_PQUEUE_FIXEDCAPACITY] = { 0 };

static inline int __stdcall compare_trees(_In_ const bntree_t child, _In_ const bntree_t parent) {
    if (child.root->data.freq > parent.root->data.freq) return -1;
}

static inline bool __cdecl pqueue_init(
    _Inout_ pqueue_t* const restrict prqueue, _In_count_(size) bntree_t* const restrict buffer, _In_ const unsigned long long size
) {
    assert(prqueue);
    prqueue->tree     = buffer;
    prqueue->count    = 0;
    prqueue->capacity = size;
    memset(prqueue->tree, 0U, sizeof(bntree_t) * size);
    return true;
}

static inline void __cdecl pqueue_clean(_Inout_ pqueue_t* const restrict prqueue) {
    assert(prqueue);
    memset(prqueue->tree, 0U, sizeof(bntree_t) * prqueue->capacity);
    memset(prqueue, 0U, sizeof(pqueue_t));
}

static inline bool __cdecl pqueue_push(_Inout_ pqueue_t* const restrict prqueue, _In_ const bntree_t data) {
    assert(prqueue);
    bntree_t           _temp     = { 0, NULL };       // NOLINT(readability-isolate-declaration)
    unsigned long long _childpos = 0, _parentpos = 0; // NOLINT(readability-isolate-declaration)
    if (prqueue->count + 1 > prqueue->capacity) {
        fputws(L"Error:: " __FUNCTIONW__ " failed because there's no more space in the pqueue_t buffer\n", stderr);
        return false; // exit(A_SPECIFIC_ERROR_CODE) ????
    }
    prqueue->count++;
    prqueue->tree[prqueue->count - 1] = data;
    _childpos                         = prqueue->count - 1;
    _parentpos                        = parent_position(_childpos);
    while ((_childpos > 0) && compare_trees(prqueue->tree[_childpos], prqueue->tree[_parentpos])) {
        _temp                     = prqueue->tree[_childpos];
        prqueue->tree[_childpos]  = prqueue->tree[_parentpos];
        prqueue->tree[_parentpos] = _temp;

        _childpos                 = _parentpos;
        _parentpos                = parent_position(_childpos);
    }
    return true;
}

static inline bool __cdecl pqueue_pop(_Inout_ pqueue_t* const restrict prqueue, _Inout_ bntree_t* const restrict popped) {
    assert(prqueue);
    assert(popped);
    const bntree_t _placeholder = { 0, NULL };

    if (!prqueue->count) {
        *popped = _placeholder;
        return false;
    }
    if (prqueue->count == 1) {
        *popped        = prqueue->tree[0];
        prqueue->count = 0;
        pqueue_clean(prqueue);
        return true;
    }
    unsigned long long _leftchildpos = 0, _rightchildpos = 0, _parentpos = 0, _pos = 0; // NOLINT(readability-isolate-declaration)
    bntree_t           _temp          = { 0, NULL };
    *popped                           = prqueue->tree[0];
    prqueue->tree[0]                  = prqueue->tree[prqueue->count - 1];
    prqueue->tree[prqueue->count - 1] = _placeholder;
    prqueue->count--;
    while (true) {
        _leftchildpos  = lchild_position(_parentpos);
        _rightchildpos = rchild_position(_parentpos);
        _pos           = (_leftchildpos <= (prqueue->count - 1)) && compare_trees(prqueue->tree[_leftchildpos], prqueue->tree[_parentpos]) ?
                             _leftchildpos :
                             _parentpos;
        if ((_rightchildpos <= (prqueue->count - 1)) && compare_trees(prqueue->tree[_rightchildpos], prqueue->tree[_pos]))
            _pos = _rightchildpos;
        if (_pos == _parentpos) break;
        _temp                     = prqueue->tree[_parentpos];
        prqueue->tree[_parentpos] = prqueue->tree[_pos];
        prqueue->tree[_pos]       = _temp;
        _parentpos                = _pos;
    }
    return true;
}

static inline bntree_t __stdcall pqueue_peek(_In_ const pqueue_t* const restrict prqueue) {
    assert(prqueue);
    const bntree_t _placeholder = { 0, NULL };
    return prqueue->tree ? prqueue->tree[0] : _placeholder;
}

#pragma endregion

#pragma region __TAILOR_MADE_BINARYTREE

typedef enum _child_kind { ROOT = 0xFF << 0x01, LEFT = 0xFF << 0x02, RIGHT = 0xFF << 0x03 } child_kind; // arms of a node

static inline bool __cdecl bntree_insert(
    _Inout_ bntree_t* const restrict tree, _Inout_opt_ btnode_t* const restrict parent, _In_ const child_kind which, _In_ const hnode_t data
) {
    assert(tree);
    btnode_t** restrict target = NULL;
    btnode_t* restrict temp    = NULL;
    if (!parent) {
        if (tree->node_count) {
            fputws(L"Error:: " __FUNCTIONW__ " does not allow insertions into a non NULL root nodes!\n", stderr);
            return false;
        }
        target = &tree->root;
    } else {
        switch (which) {
            case LEFT :
                if (parent->left) {
                    fputws(L"Error:: " __FUNCTIONW__ " does not allow insertions into a non NULL child nodes!\n", stderr);
                    return false;
                }
                target = &parent->left;
                break;
            case RIGHT :
                if (parent->right) {
                    fputws(L"Error:: " __FUNCTIONW__ " does not allow insertions into a non NULL child nodes!\n", stderr);
                    return false;
                }
                target = &parent->right;
                break;
            default : break;
        }
    }
    if (!(temp = (btnode_t*) malloc(sizeof(btnode_t)))) { // NOLINT(bugprone-assignment-in-if-condition) if the allocation fails
        fputws(L"Error:: malloc failed inside " __FUNCTIONW__ "\n", stderr);
        return false;
    }
    temp->data = data;
    temp->left = temp->right = NULL;
    *target                  = temp;
    tree->node_count++;
    return true;
}

static inline bool __cdecl bntree_remove( // NOLINT(misc-no-recursion)
    _Inout_ bntree_t* const restrict tree,
    _Inout_opt_ btnode_t* const restrict parent,
    _In_ const child_kind which
) {
    assert(tree);

    if (!tree->node_count) {
        fputws(L"Error:: " __FUNCTIONW__ " cannot remove nodes from an empty binary tree\n", stderr);
        return false;
    }

    btnode_t** restrict target = NULL;

    if (!parent)
        target = &tree->root;
    else {
        switch (which) {
            case LEFT  : target = &parent->left; break;
            case RIGHT : target = &parent->right; break;
            default    : break;
        }
    }

    if (*target) {
        bntree_remove(tree, *target, LEFT);
        bntree_remove(tree, *target, RIGHT);

        free(*target);
        *target = NULL;
        tree->node_count--;
    }

    return true;
}

static inline bntree_t __cdecl bntree_merge(
    _In_ bntree_t* const restrict left, _In_ bntree_t* const restrict right, _In_ const hnode_t data
) {
    assert(left);
    assert(right);

    bntree_t merged = { 0, NULL };
    if (!bntree_insert(&merged, NULL, ROOT, data)) {
        fputws(L"Error:: ", stderr);
        return merged;
    }

    merged.root->left   = left->root;
    merged.root->right  = right->root;
    merged.node_count  += left->node_count + right->node_count;

    left->root = right->root = NULL;
    left->node_count = right->node_count = 0;

    return merged;
}

#pragma endregion

// the goal of Huffman encoding is to represent symbols that occur more frequently with fewer bits than the symbols that occur less
// frequently - a concept known as minimum entropy coding
// the "symbol" here can be anything but is usually a byte!

// entropy E of a symbol S is defined as E(S) = -log2(P(S))
// where P is the probability of the select symbol being found in the given buffer
// e.g. "ABCBCBCJKUGRFCCCSYJIOIHICCC"
// frequency of the character 'C' in the above string is 9
// total number of characters in that string is 27
// P('C') = 9/27
// E('C') = -log2(P(9/27))
//        = 1.58496250072116    - this is the entropy of the character 'C' in the given string
// in theory, we can respresent 'C' in this string using 1.58496250072116 bits instead of the conventional 8 bits
// since the character 'C' occurs 9 times, the its cumulative contribution to the string's entropy is
// = 9 x 1.58496250072116
// = 14.2646625064904
// again, in theory all the 'C' characters in the above string can be represented by a total of 14.2646625064904 bits

#define _BYTEFREQ_LEN_FIXED 256LLU

// the first step in Huffman encoding is the determination of symbol frequencies
static inline void __cdecl calculate_byte_frequencies(
    _In_ const unsigned char* const restrict inbuffer,
    _In_ const unsigned long long size,
    _Inout_count_(_BYTEFREQ_LEN_FIXED) unsigned long long* const restrict frequencies // could be a stack based or heap allocated array
) {
    assert(inbuffer);
    assert(size);
    memset(frequencies, 0U, sizeof(unsigned long long) * (_BYTEFREQ_LEN_FIXED));
    for (unsigned long long i = 0; i < size; ++i) frequencies[inbuffer[i]]++;
}

static inline bntree_t __cdecl build_huffman_tree() { }

static inline int64_t __cdecl compress(
    _In_ const unsigned char* const restrict inbuffer, _Inout_ unsigned char* const restrict outbuffer, _In_ const unsigned long long size
) {
    //
}

static inline int64_t __cdecl decompress(
    _In_ const unsigned char* const restrict inbuffer, _Inout_ unsigned char* const restrict outbuffer, _In_ const unsigned long long size
) {
    //
}
