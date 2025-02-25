#pragma once

// clang-format off
    #define _AMD64_
    #define WIN32_LEAN_AND_MEAN
    #define WIN32_EXTRA_MEAN
    // UCRT headers, for some freaking reason, use a plain #if predicate for __STDC_WANT_SECURE_LIB__
    // not an #ifdef predicate, so we need to provide a valued definition for __STDC_WANT_SECURE_LIB__,
    // a plain #define results in compile time error
    #define __STDC_WANT_SECURE_LIB__ 1
    #define NOMINMAX     // it seems that only <Windows.h> has the internal header guards receptive to NOMINMAX
    // if we include system headers directly without relying on <Windows.h> for transient includes, #define NOMINMAX offers no help! YIKES!
    #include <windef.h>
    #include <errhandlingapi.h>
    #include <fileapi.h>
    #include <handleapi.h>
// clang-format on

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>

// static_assert(max(10, 0), "see max is reachable here!");
#if defined(min) && defined(max)
    #undef min
    #undef max
#endif

#ifdef __cplusplus
    #define _TRIPLE_UNDERSCORE_PREFIX(name) ___##name
#else
    #define _TRIPLE_UNDERSCORE_PREFIX(name) name
#endif // __cplusplus

// corecrt_io.h also has open & write functions defined as deprecated POSIX extensions inside an #ifdef __cplusplus block,
// which leads to name collision when compiled as C++ for testing, hence the conditional prefixing
[[nodiscard]] static inline unsigned char* __cdecl _TRIPLE_UNDERSCORE_PREFIX(open)(
    _In_ const wchar_t* const restrict filepath, _Inout_ unsigned long* const restrict nbytes
) {
    assert(filepath);
    assert(nbytes);

    *nbytes               = 0;
    const HANDLE64 hFile  = CreateFileW(filepath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    unsigned char* buffer = NULL; // DO NOT REFACTOR THIS AS AN SINGLE INLINE DEFINITION AT LINE 63
    // C++ DOES NOT ALLOW goto TO JUMP OVER UNINITIALIZED VARIABLES!!!

    if (hFile == INVALID_HANDLE_VALUE) [[unlikely]] {
        fwprintf_s(stderr, L"Error %lu in CreateFileW while opening %s\n", GetLastError(), filepath);
        return NULL;
    }

    LARGE_INTEGER fsize = { .QuadPart = 0 };
    if (!GetFileSizeEx(hFile, &fsize)) [[unlikely]] {
        fwprintf_s(stderr, L"Error %lu in GetFileSizeEx while opening %s\n", GetLastError(), filepath);
        goto PREMATURE_RETURN;
    }

    buffer = (unsigned char*) malloc(fsize.QuadPart); // caller is responsible for freeing this buffer.
    if (!buffer) [[unlikely]] {
        fputws(L"Memory allocation error: malloc returned NULL", stderr);
        goto PREMATURE_RETURN;
    }

    if (!ReadFile(hFile, buffer, fsize.QuadPart, nbytes, NULL)) [[unlikely]] {
        fwprintf_s(stderr, L"Error %lu in ReadFile while opening %s\n", GetLastError(), filepath);
        free(buffer);
        goto PREMATURE_RETURN;
    }

    CloseHandle(hFile);
    return buffer;

PREMATURE_RETURN:
    CloseHandle(hFile);
    return NULL;
}

static inline bool __cdecl _TRIPLE_UNDERSCORE_PREFIX(write)(
    _In_ const wchar_t* const restrict filepath, _In_reads_(size) const unsigned char* const restrict buffer, _In_ const unsigned long size
) {
    assert(filepath); // ??
    assert(buffer);

    // this is void* const
    const HANDLE64 hfile = CreateFileW(filepath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfile == INVALID_HANDLE_VALUE) [[unlikely]] {
        fwprintf_s(stderr, L"Error %lu in CreateFileW while creating %s\n", GetLastError(), filepath);
        return false;
    }

    unsigned long nbyteswritten = 0;
    if (!WriteFile(hfile, buffer, size, &nbyteswritten, NULL)) [[unlikely]] {
        fwprintf_s(stderr, L"Error %lu in WriteFile while creating %s\n", GetLastError(), filepath);
        CloseHandle(hfile);
        return false;
    }

    CloseHandle(hfile);
    return true;
}

#undef _TRIPLE_UNDERSCORE_PREFIX
