#pragma once

// <utilities.h> will get transiently included by other headers

#include <bintree.h>
#include <bitops.h>
#include <compression.h>
#include <fileio.h>
#include <pqueue.h>

// the problem with the data structures in these headers is that they use type erasure for the sake of supporting arbitraty user defined types
// we lose a lot because this requires storing type erased heap allocated pointers instead of directly storing the values
// but this would make these data structures tightly coupled to a single type, hence losing their "library" status

// since this is not important to us and we need maximum performance, we'll make variants tailor made for huffman algorithms
// embedding associated user defined types directly inside the data structures, leveraging the tight coupling to eliminate
// type erasure, gratuitous heap allocations and maximize stack usage