#pragma once

#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/fcntl.h>
#include <sys/stat.h>

#if defined(__TEST__) && defined(__VERBOSE_TEST_IO__)
    #define dbgprinf(...) fprintf(stderr, __VA_ARGS__)
#else
    #define dbgprinf(...)
#endif

#define HELPER(expression) #expression
#define TO_STR(expression) HELPER(expression)

// retruns the offset of the parent node
static inline unsigned long long __attribute__((__always_inline__)) parent_position(const unsigned long long child) {
    return !child ? 0 : (child - 1) / 2 /* deliberate truncating division. */;
}

// retruns the offset of the left child node
static inline unsigned long long __attribute__((__always_inline__)) lchild_position(const unsigned long long parent) {
    return parent * 2 + 1;
}

// retruns the offset of the left right node
static inline unsigned long long __attribute__((__always_inline__)) rchild_position(const unsigned long long parent) {
    return parent * 2 + 2;
}
