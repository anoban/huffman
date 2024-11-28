#pragma once
#include <fileio.h>
#include <stdint.h>
#include <stdlib.h>
#include <utilities.h>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
// the problem with the data structures in the other headers inside ./include/ is that they liberally rely on type erasure for the sake of supporting arbitraty user defined types
// we lose a lot of type safety and performance here because this requires storing type erased heap allocated pointers instead of directly storing the values
// as this would make these data structures tightly coupled to a single type, hence losing their versatility

// since this is not important to us and we need maximum performance, we'll make variants tailor made for huffman algorithms
// embedding associated user defined types directly inside the data structures, leveraging the tight coupling to eliminate
// type erasure, gratuitous heap allocations and maximize stack usage
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

#define BYTECOUNT                          (256LLU)
#define GLOBAL_BTNODE_BUFFER_FIXEDCAPACITY (1LLU << 10) // number of btnode_t s a stack based priority queue can accomodate

// represents a Huffman node.
typedef struct _hnode {
        unsigned long long symbol; // will be (0, UCHAR_MAX) for leaf nodes, and will be UINT32_MAX for others
        unsigned long long frequency;
} hnode_t;

static_assert(sizeof(hnode_t) == 16);
static_assert(offsetof(hnode_t, symbol) == 0);
static_assert(offsetof(hnode_t, frequency) == 8);

// represents a binary tree node, this is the type that will be used to build the Huffman tree using a priority queue
typedef struct _btnode {
        struct _btnode* left;
        struct _btnode* right;
        hnode_t         data;
} btnode_t;

static_assert(sizeof(btnode_t) == 16 + sizeof(hnode_t));
static_assert(offsetof(btnode_t, left) == 0);
static_assert(offsetof(btnode_t, right) == 8);
static_assert(offsetof(btnode_t, data) == 16);

// represents a binary tree, to represent the Huffman tree
typedef struct _bintree {
        btnode_t*          tree;
        btnode_t*          root;
        unsigned long long node_count;
} bntree_t;

static_assert(sizeof(bntree_t) == 24);
static_assert(offsetof(bntree_t, tree) == 0);
static_assert(offsetof(bntree_t, root) == 8);
static_assert(offsetof(bntree_t, node_count) == 16);

// represents a Huffman code
typedef struct _hcode {
        bool           is_used; // ???????
        unsigned char  length;
        unsigned short code;
} hcode_t; // no padding yeehaw :)

static_assert(sizeof(hcode_t) == 4);
static_assert(offsetof(hcode_t, is_used) == 0);
static_assert(offsetof(hcode_t, length) == 1);
static_assert(offsetof(hcode_t, code) == 2);

typedef struct _pqueue {
        unsigned  count;
        unsigned  capacity;
        btnode_t* tree;
} pqueue_t;

static_assert(sizeof(pqueue_t) == 16);
static_assert(offsetof(pqueue_t, count) == 0);
static_assert(offsetof(pqueue_t, capacity) == 4);
static_assert(offsetof(pqueue_t, tree) == 8);

//-------------------------------------------------------------------------------------------------------------------------------//
//                                                      MISCELLANEOUS PRELIMINARIES                                              //
//-------------------------------------------------------------------------------------------------------------------------------//

static inline void __cdecl scan_frequencies( // the first step in Huffman encoding is the determination of symbol frequencies
    _In_bytecount_(size) const unsigned char* const restrict buffer,
    _In_ const unsigned long long size,
    _Inout_count_(BYTECOUNT) unsigned long long* const restrict frequencies // could be a stack based or heap allocated array
) {
    assert(buffer);
    assert(size);
    memset(frequencies, 0U, sizeof(typeof_unqual(*frequencies)) * BYTECOUNT);
    for (unsigned long long i = 0; i < size; ++i) frequencies[buffer[i]]++;
}

//-------------------------------------------------------------------------------------------------------------------------------//
//                  AN ALTERNATIVE IMPLEMENTATION OF PRIORITY QUEUE THAT USES STACK FOR BETTER PERFORMANCE                       //
//-------------------------------------------------------------------------------------------------------------------------------//

[[nodiscard]] static inline bool __stdcall compare(_In_ const btnode_t child, _In_ const btnode_t parent) {
    return child.data.frequency < parent.data.frequency; // we need the priority queue to yield the node with smallest frequency first
    // hence the less than operator
}

[[nodiscard]] static inline pqueue_t __stdcall pqueue_init(
    /* expects an array on the stack because pqueue_clean() will not free() this buffer */
    _In_count_(node_count) btnode_t* const restrict buffer,
    _In_ const unsigned long long node_count
) {
    assert(buffer);

    memset(buffer, 0U, sizeof(typeof_unqual(*buffer)) * node_count); // zero out the binary tree node buffer
    pqueue_t prqueue = { .count = 0, .capacity = (unsigned) node_count, .tree = buffer };
    // (pqueue_t) { .tree = buffer, .count = 0, .capacity = node_count }; this syntax is invalid in C++, yikes!
    return prqueue;
}

static inline void __stdcall pqueue_clean(_Inout_ pqueue_t* const restrict prqueue) {
    assert(prqueue);
    memset(prqueue->tree, 0U, sizeof(typeof_unqual(*prqueue->tree)) * prqueue->capacity); // cleanup the buffer
    memset(prqueue, 0U, sizeof(typeof_unqual(*prqueue)));
}

static inline bool __stdcall pqueue_push(_Inout_ pqueue_t* const restrict prqueue, _In_ const btnode_t data) {
    assert(prqueue);

    btnode_t           _temp     = { 0 };
    unsigned long long _childpos = 0, _parentpos = 0; // NOLINT(readability-isolate-declaration)

    if (prqueue->count + 1 > prqueue->capacity) [[unlikely]] {
        fputws(L"Error:: " __FUNCTIONW__ " failed because there's no more space in the pqueue_t buffer\n", stderr);
        return false;
    }

    prqueue->count++;
    prqueue->tree[prqueue->count - 1] = data;
    _childpos                         = prqueue->count - 1;
    _parentpos                        = parent_position(_childpos);

    while ((_childpos > 0) && compare(prqueue->tree[_childpos], prqueue->tree[_parentpos])) {
        _temp                     = prqueue->tree[_childpos];
        prqueue->tree[_childpos]  = prqueue->tree[_parentpos];
        prqueue->tree[_parentpos] = _temp;

        _childpos                 = _parentpos;
        _parentpos                = parent_position(_childpos);
    }

    return true;
}

static inline bool __stdcall pqueue_pop(_Inout_ pqueue_t* const restrict prqueue, _Inout_ btnode_t* const restrict popped) {
    assert(prqueue);
    assert(popped);

    const btnode_t _placeholder = { 0 };

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
    btnode_t           _temp          = { 0 };
    *popped                           = prqueue->tree[0];
    prqueue->tree[0]                  = prqueue->tree[prqueue->count - 1];
    prqueue->tree[prqueue->count - 1] = _placeholder;
    prqueue->count--;

    while (true) {
        _leftchildpos  = lchild_position(_parentpos);
        _rightchildpos = rchild_position(_parentpos);
        _pos = (_leftchildpos <= (prqueue->count - 1)) && compare(prqueue->tree[_leftchildpos], prqueue->tree[_parentpos]) ? _leftchildpos :
                                                                                                                             _parentpos;
        if ((_rightchildpos <= (prqueue->count - 1)) && compare(prqueue->tree[_rightchildpos], prqueue->tree[_pos])) _pos = _rightchildpos;
        if (_pos == _parentpos) break;
        _temp                     = prqueue->tree[_parentpos];
        prqueue->tree[_parentpos] = prqueue->tree[_pos];
        prqueue->tree[_pos]       = _temp;
        _parentpos                = _pos;
    }
    return true;
}

static inline btnode_t __stdcall pqueue_peek(_In_ const pqueue_t* const restrict prqueue) {
    assert(prqueue);

    const btnode_t _placeholder = { 0 };
    return prqueue->tree ? prqueue->tree[0] : _placeholder;
}

//-------------------------------------------------------------------------------------------------------------------------------//
//                                          ROUTINES FOR HUFFMAN TREE BUILDING                                                   //
//-------------------------------------------------------------------------------------------------------------------------------//

static inline bntree_t __cdecl build_huffman_tree(
    _In_reads_(BYTECOUNT) const unsigned long long* const restrict frequencies,
    _Inout_count_(GLOBAL_BTNODE_BUFFER_FIXEDCAPACITY) btnode_t* const restrict pqueue_nodebuffer,
    _Inout_count_(GLOBAL_BTNODE_BUFFER_FIXEDCAPACITY) btnode_t* const restrict bntree_nodebuffer
) {
    assert(frequencies);
    assert(pqueue_nodebuffer);
    assert(bntree_nodebuffer);

    pqueue_t           prqueue = pqueue_init(pqueue_nodebuffer, GLOBAL_BTNODE_BUFFER_FIXEDCAPACITY);
    btnode_t           temp = { 0 }, aggregate = { 0 };
    bntree_t           huffman                         = { 0 }; // Huffman tree
    unsigned long long nsymbols_with_nonzero_frequency = 0, write_caret = 0;

    // first push all the leaf nodes into the priority queue
    for (unsigned i = 0; i < BYTECOUNT; ++i) {
        if (frequencies[i]) { // only entertain the bytes with non zero frequencies
            temp.left = temp.right = NULL;
            temp.data.symbol       = i;
            temp.data.frequency    = frequencies[i];

            if (!pqueue_push(&prqueue, temp)) [[unlikely]] {
                // TODO
            }
            nsymbols_with_nonzero_frequency++; // register the number of bytes with non-zero frequencies
        }
    }

    dbgwprinf_s(L"There were %4llu unique symbols in this buffer\n", nsymbols_with_nonzero_frequency);
    temp.data.symbol = temp.data.frequency = 0; // .left and .right are already set to NULL

    // bootstrap the binary tree
    huffman.tree                           = bntree_nodebuffer; // take ownership of the buffer

    // now to the actual Huffman tree building
    while (pqueue_pop(&prqueue, &temp) // || pqueue_pop(&prqueue, &popped_02)
           // when the first call returns true the programme never evaluates the second call, FUCKING SHORTCIRCUITING HEH :(
    ) { // while the priority queue is not empty, take nodes one by one and start building the Huffman tree
        dbgwprinf_s(L"%10llX - %10llu\n", temp.data.symbol, temp.data.frequency);

        // when the priority queue is empty, pqueue_pop() will update the popped struct to be an empty struct so we do not need to
        // do that at the end of the loop manually

        huffman.tree[write_caret++] = temp; // copy the popped node to the tree's buffer
        huffman.node_count++;               // document the copy

        if (pqueue_pop(&prqueue, &temp)) { // pop another node to pair with the previous node
            dbgwprinf_s(L"%10llX - %10llu\n", temp.data.symbol, temp.data.frequency);

            huffman.tree[write_caret++] = temp; // copy the popped node to the tree's buffer
            huffman.node_count++;               // document the copy

            // make the third node, with the combined frequency of the two popped nodes
            // Huffman tree is left balanced, so the smallest node goes to the left
            aggregate.left        = huffman.tree + write_caret - 2; // popped first, the smallest
            aggregate.right       = huffman.tree + write_caret - 1; // popped second, the next smallest
            aggregate.data.symbol = UINT32_MAX;                     // this a marker that registers that this is not a leaf node
            aggregate.data.frequency =
                aggregate.left->data.frequency + aggregate.right->data.frequency; // cumulative frequency of the two child nodes

            if (!pqueue_push(&prqueue, aggregate)) [[unlikely]] { // push the new non-leaf node into the priority queue
                // TODO
            }
        }
    }

    return huffman;
}

// the goal of Huffman encoding is to represent symbols that occur more frequently with fewer bits than the symbols that occur less
// frequently - a concept known as minimum entropy coding
// the "symbol" here can be anything but is usually a byte!

// entropy E of a symbol S is defined as E(S) = -log2(P(S))
// where P(S) is the probability of the symbol S in the given buffer e.g. "ABCBCBCJKUGRFCCCSYJIOIHICCC"
// frequency of the character 'C' in the above string is 9, total number of characters is 27
// P('C') = 9/27
// E('C') = -log2(P(9/27))
//        = 1.58496250072116 - this is the entropy of one character 'C' in the given string

// in theory, we can respresent 'C' in this string using 1.58496250072116 bits instead of the conventional 8 bits
// since the character 'C' occurs 9 times, the its cumulative entropy in the string is
// = 9 x 1.58496250072116
// = 14.2646625064904
// again, in theory all the 'C' characters in the above string can be represented by a total of 14.2646625064904 bits

static inline unsigned long long __cdecl compress(
    _In_ const unsigned char* const restrict inbuffer, _Inout_ unsigned char* const restrict outbuffer, _In_ const unsigned long long size
) {
    //
}

static inline unsigned long long __cdecl decompress(
    _In_ const unsigned char* const restrict inbuffer, _Inout_ unsigned char* const restrict outbuffer, _In_ const unsigned long long size
) {
    //
}
