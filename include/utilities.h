#pragma once
#include <stdint.h>

#ifdef __cplusplus // for C++ compatibility
    #define _CXX_COMPAT_REINTERPRET_CAST(type, identifier) (reinterpret_cast<type>((identifier)))
    #define _CXX_COMPAT_CONST_CAST(type, identifier)       (const_cast<type>((identifier)))
#else
    #define _CXX_COMPAT_REINTERPRET_CAST(type, identifier) (identifier)
    #define _CXX_COMPAT_CONST_CAST(type, identifier)       (identifier)
#endif // __cplusplus

#if defined(_DEBUG) || defined(DEBUG)
    #define dbgwprinf_s(...) wprintf_s(__VA_ARGS__)
#else
    #define dbgwprinf_s(...)
#endif

static __forceinline size_t __stdcall parent_position(_In_ register const size_t child) {
    return !child ? 0 : (child - 1) / 2 /* deliberate truncating division. */;
}

static __forceinline size_t __stdcall lchild_position(_In_ register const size_t parent) { return parent * 2 + 1; }

static __forceinline size_t __stdcall rchild_position(_In_ register const size_t parent) { return parent * 2 + 2; }
