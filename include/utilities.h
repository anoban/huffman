#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus // for C++ compatibility
    #define _CXX_COMPAT_REINTERPRET_CAST(type, identifier) (reinterpret_cast<type>((identifier)))
    #define _CXX_COMPAT_CONST_CAST(type, identifier)       (const_cast<type>((identifier)))
    #define _CXX_COMPAT_CONLY_KEYWORD_GUARD(keyword)
    #define typeof(...) decltype(__VA_ARGS__)
#else
    #define _CXX_COMPAT_REINTERPRET_CAST(type, identifier) (identifier)
    #define _CXX_COMPAT_CONST_CAST(type, identifier)       (identifier)
    #define _CXX_COMPAT_CONLY_KEYWORD_GUARD(keyword)       keyword
#endif // __cplusplus

#if defined(__TEST__) && defined(__VERBOSE_TEST_IO__)
    #define gtstwprinf_s(...) wprintf_s(__VA_ARGS__)
#else
    #define gtstwprinf_s(...)
#endif

// retruns the offset of the parent node
static __forceinline size_t __stdcall parent_position(_In_ register const size_t child) {
    return !child ? 0 : (child - 1) / 2 /* deliberate truncating division. */;
}

// retruns the offset of the left child node
static __forceinline size_t __stdcall lchild_position(_In_ register const size_t parent) { return parent * 2 + 1; }

// retruns the offset of the left right node
static __forceinline size_t __stdcall rchild_position(_In_ register const size_t parent) { return parent * 2 + 2; }
