#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// these helpers make the codebase ugly, so opting for plain C style casts
#ifdef __cplusplus // for the sake of C++ compatibility
    #define _CXX_COMPAT_REINTERPRET_CAST(type, identifier) (reinterpret_cast<type>((identifier)))
    #define _CXX_COMPAT_CONST_CAST(type, identifier)       (const_cast<type>((identifier)))
    #define _CXX_COMPAT_STDC_KEYWORD_GUARD(keyword)
#else
    #define _CXX_COMPAT_REINTERPRET_CAST(type, identifier) (identifier)
    #define _CXX_COMPAT_CONST_CAST(type, identifier)       (identifier)
    #define _CXX_COMPAT_STDC_KEYWORD_GUARD(keyword)        keyword
#endif // __cplusplus

#if defined(__TEST__) && defined(__VERBOSE_TEST_IO__)
    #define gtstwprinf_s(...) wprintf_s(__VA_ARGS__)
#else
    #define gtstwprinf_s(...)
#endif

#define HELPER(expression) #expression
#define STR(expression)    HELPER(expression)

// retruns the offset of the parent node
static inline unsigned long long __stdcall parent_position(_In_ const unsigned long long child) {
    return !child ? 0 : (child - 1) / 2 /* deliberate truncating division. */;
}

// retruns the offset of the left child node
static inline unsigned long long __stdcall lchild_position(_In_ const unsigned long long parent) { return parent * 2 + 1; }

// retruns the offset of the left right node
static inline unsigned long long __stdcall rchild_position(_In_ const unsigned long long parent) { return parent * 2 + 2; }
