#pragma once
#ifndef __BITIO_H__
    #define __BITIO_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #define _AMD64_ // architecture
    #define WIN32_LEAN_AND_MEAN
    #define WIN32_EXTRA_MEAN
#endif

#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <sal.h>

static inline uint8_t* open(_In_ const wchar_t* restrict file_name, _Inout_ uint64_t* const nread_bytes) {
    *nread_bytes    = 0;
    void *   handle = NULL, *buffer = NULL;

    handle          = CreateFileW(file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in CreateFileW\n", GetLastError());
        return NULL;
    }

    LARGE_INTEGER fsize;
    if (!GetFileSizeEx(handle, &fsize)) {
        fwprintf_s(stderr, L"Error %lu in GetFileSizeEx\n", GetLastError());
        goto errexit;
    }

    buffer          = malloc(fsize.QuadPart); // caller is responsible for freeing this buffer.
    if (!buffer) {
        fputws(L"Memory allocation error: malloc returned NULL", stderr);
        goto errexit;
    }

    if (ReadFile(handle, buffer, fsize.QuadPart, nread_bytes, NULL)) {
        CloseHandle(handle);
        return buffer;
    } else {
        fwprintf_s(stderr, L"Error %lu in ReadFile\n", GetLastError());
        free(buffer);
        goto errexit;
    }
         
errexit: 
        CloseHandle(handle);
        return NULL;
} 

#endif // !__BITIO_H__
