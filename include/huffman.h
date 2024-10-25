#pragma once
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
        uint8_t byte; // only applicable to leaf nodes
        // could use a smaller type for marker but the member freq inside hnode_t will warrant padding, so why not use that here
        size_t  marker; // a value (UINT32_MAX) registering a non-leaf node
} symbol_t;

static_assert(sizeof(symbol_t) == 8);
static_assert(!offsetof(symbol_t, byte));
static_assert(!offsetof(symbol_t, marker));

typedef struct _hnode {  // represents a Huffman node.
        symbol_t symbol; // byte for a leaf node, UINT32_MAX for internodes
        size_t   freq;
} hnode_t;

static_assert(sizeof(hnode_t) == 16);
static_assert(!offsetof(hnode_t, symbol));
static_assert(offsetof(hnode_t, freq) == 8);

typedef struct _hcode { // represents a Huffman code
        bool     is_used;
        uint8_t  length;
        uint16_t code;
} hcode_t; // no padding yeehaw :)

static_assert(sizeof(hcode_t) == 4);
static_assert(!offsetof(hcode_t, is_used));
static_assert(offsetof(hcode_t, length) == 1);
static_assert(offsetof(hcode_t, code) == 2);

#pragma endregion

#pragma region __TAILOR_MADE_PQUEUE

typedef struct _pqueue {
        uint32_t count;
        uint32_t capacity;
        hnode_t* tree;
} pqueue;

static_assert(sizeof(pqueue) == 16);
static_assert(offsetof(pqueue, count) == 0);
static_assert(offsetof(pqueue, capacity) == 4);
static_assert(offsetof(pqueue, tree) == 8);

#define DEFAULT_PQUEUE_CAPACITY       (3LLU << 10)
// we are storing the actual structs instead of pointers here
#define DEFAULT_PQUEUE_CAPACITY_BYTES (DEFAULT_PQUEUE_CAPACITY * sizeof(hnode_t)) // around 48 kilobytes
static hnode_t __PQUEUE_GLOBAL_BUFFER[DEFAULT_PQUEUE_CAPACITY] = { 0 };

[[nodiscard]] static inline bool __stdcall compare_hnode(_In_ const hnode_t child, _In_ const hnode_t parent) {
    // TODO
}

[[nodiscard]] static inline bool __cdecl pqueue_init(_Inout_ pqueue* const restrict prqueue) {
    assert(prqueue);

    prqueue->tree     = __PQUEUE_GLOBAL_BUFFER;
    prqueue->count    = 0;
    prqueue->capacity = DEFAULT_PQUEUE_CAPACITY;
    memset(prqueue->tree, 0U, sizeof(__PQUEUE_GLOBAL_BUFFER));
    return true;
}

static inline void __cdecl pqueue_clean(_Inout_ pqueue* const restrict prqueue) {
    assert(prqueue);
    memset(prqueue->tree, 0U, sizeof(__PQUEUE_GLOBAL_BUFFER)); // zero out the global buffer
    memset(prqueue, 0U, sizeof(pqueue));
}

[[nodiscard]] static inline bool __cdecl pqueue_push(_Inout_ pqueue* const restrict prqueue, _In_ const hnode_t data) {
    assert(prqueue);

    hnode_t _temp_node = { .symbol.marker = 0, .freq = 0 }; // NOLINT(readability-isolate-declaration)
    size_t  _childpos = 0, _parentpos = 0;                  // NOLINT(readability-isolate-declaration)

    if (prqueue->count + 1 > prqueue->capacity) {
        fputws(L"Error:: " __FUNCTIONW__ " failed because there's no more space in the static pqueue buffer\n", stderr);
        return false; // exit(A_SPECIFIC_ERROR_CODE) ????
    }

    prqueue->count++;
    prqueue->tree[prqueue->count - 1] = data;
    _childpos                         = prqueue->count - 1;
    _parentpos                        = parent_position(_childpos);

    while ((_childpos > 0) && compare_hnode(prqueue->tree[_childpos], prqueue->tree[_parentpos])) {
        _temp_node                = prqueue->tree[_childpos];
        prqueue->tree[_childpos]  = prqueue->tree[_parentpos];
        prqueue->tree[_parentpos] = _temp_node;

        _childpos                 = _parentpos;
        _parentpos                = parent_position(_childpos);
    }

    return true;
}

[[nodiscard]] static inline bool __cdecl pqueue_pop(_Inout_ pqueue* const restrict prqueue, _Inout_ hnode_t* const restrict popped) {
    assert(prqueue);
    assert(popped);

    if (!prqueue->count) {
        *popped = (hnode_t) { .symbol.marker = 0, .freq = 0 };
        return false;
    }

    if (prqueue->count == 1) {
        *popped        = prqueue->tree[0];
        prqueue->count = 0;
        pqueue_clean(prqueue);
        return true;
    }

    size_t  _leftchildpos = 0, _rightchildpos = 0, _parentpos = 0, _pos = 0; // NOLINT(readability-isolate-declaration)
    hnode_t _temp                     = { .symbol.marker = 0, .freq = 0 };

    *popped                           = prqueue->tree[0];

    prqueue->tree[0]                  = prqueue->tree[prqueue->count - 1];
    prqueue->tree[prqueue->count - 1] = (hnode_t) { .symbol.marker = 0, .freq = 0 };
    prqueue->count--;

    while (true) {
        _leftchildpos  = lchild_position(_parentpos);
        _rightchildpos = rchild_position(_parentpos);

        _pos           = (_leftchildpos <= (prqueue->count - 1)) && compare_hnode(prqueue->tree[_leftchildpos], prqueue->tree[_parentpos]) ?
                             _leftchildpos :
                             _parentpos;
        if ((_rightchildpos <= (prqueue->count - 1)) && compare_hnode(prqueue->tree[_rightchildpos], prqueue->tree[_pos]))
            _pos = _rightchildpos;
        if (_pos == _parentpos) break;

        _temp                     = prqueue->tree[_parentpos];
        prqueue->tree[_parentpos] = prqueue->tree[_pos];
        prqueue->tree[_pos]       = _temp;

        _parentpos                = _pos;
    }

    return true;
}

static inline hnode_t __stdcall pqueue_peek(_In_ const pqueue* const restrict prqueue) {
    return prqueue->tree ? prqueue->tree[0] : (hnode_t) { .symbol.marker = 0, .freq = 0 };
}

#pragma endregion

#pragma region __TAILOR_MADE_BINARYTREE

typedef struct _btnode {
        struct _btnode* left;
        struct _btnode* right;
        hnode_t         data;
} btnode;

static_assert(sizeof(btnode) == 16 + sizeof(hnode_t));
static_assert(offsetof(btnode, left) == 0);
static_assert(offsetof(btnode, right) == 8);
static_assert(offsetof(btnode, data) == 16);

typedef struct _bintree {
        size_t  node_count;
        btnode* root;
} bntree;

static_assert(sizeof(bntree) == 16);
static_assert(offsetof(bntree, node_count) == 0);
static_assert(offsetof(bntree, root) == 8);

typedef enum _child_kind { ROOT = 0xFF << 0x01, LEFT = 0xFF << 0x02, RIGHT = 0xFF << 0x03 } child_kind; // arms of a node

static inline bool __cdecl bntree_insert(
    _Inout_ bntree* const restrict tree,
    _Inout_opt_ btnode* const restrict parent,
    _In_ const child_kind which,
    _In_ void* const restrict data
) {
    assert(tree);
    assert(data);

    btnode** restrict target = NULL;
    btnode* restrict temp    = NULL;

    if (!parent) {
        if (tree->node_count) {
            fputws(L"Error:: " __FUNCTIONW__ " does not allow insertions into a non NULL root nodes!\n", stderr);
            return false;
        }
        target = &tree->root;

    } else {
        switch (which) {
            case LEFT :
                {
                    if (parent->left) {
                        fputws(L"Error:: " __FUNCTIONW__ " does not allow insertions into a non NULL child nodes!\n", stderr);
                        return false;
                    }
                    target = &parent->left;
                    break;
                }
            case RIGHT :
                {
                    if (parent->right) {
                        fputws(L"Error:: " __FUNCTIONW__ " does not allow insertions into a non NULL child nodes!\n", stderr);
                        return false;
                    }
                    target = &parent->right;
                    break;
                }
            default : break;
        }
    }

    if (!(temp = (btnode*) malloc(sizeof(btnode)))) { // NOLINT(bugprone-assignment-in-if-condition) if the allocation fails
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
    _Inout_ bntree* const restrict tree,
    _Inout_opt_ btnode* const restrict parent,
    _In_ const child_kind which
) {
    assert(tree);

    if (!tree->node_count) {
        fputws(L"Error:: " __FUNCTIONW__ " cannot remove nodes from an empty binary tree\n", stderr);
        return false;
    }

    btnode** restrict target = NULL;

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

static inline bntree __cdecl bntree_merge(
    _In_ bntree* const restrict left, _In_ bntree* const restrict right, _In_ void* const restrict data
) {
    assert(left);
    assert(right);
    assert(data);

    bntree merged = { .node_count = 0, .root = NULL };
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

#define BYTEFREQ_ARRAY_SIZE 256LLU

static __forceinline bool __cdecl EnumerateBytes(
    _In_ const uint8_t* const restrict inbuffer,
    _Inout_count_(BYTEFREQ_ARRAY_SIZE) size_t* const restrict frequencies, // could be a stack based or heap allocated array
    _In_ const size_t size
) {
    assert(inbuffer);
    assert(size);
    // assumes `frequencies` to be a zeroed out array
    memset(frequencies, 0U, sizeof(size_t) * (BYTEFREQ_ARRAY_SIZE));
    for (size_t i = 0; i < size; ++i) frequencies[inbuffer[i]]++;
}

static inline int64_t __cdecl Compress(
    _In_ const uint8_t* const restrict inbuffer, _Inout_ uint8_t* const restrict outbuffer, _In_ const size_t size
) {
    //
}

static inline int64_t __cdecl Decompress(
    _In_ const uint8_t* const restrict inbuffer, _Inout_ uint8_t* const restrict outbuffer, _In_ const size_t size
) {
    //
}
