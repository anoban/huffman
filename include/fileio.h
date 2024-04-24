#pragma once
#ifndef __FILEIO_H__
    #define __FILEIO_H__

    #include <stdbool.h>
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

    #pragma comment(lib, "User32.lib")

static inline uint8_t* openFile(_In_ const wchar_t* restrict file_name, _Inout_ size_t* const nread_bytes) {
    *nread_bytes             = 0;
    const HANDLE const hFile = CreateFileW(file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in CreateFileW\n", GetLastError());
        return NULL;
    }

    LARGE_INTEGER fsize;
    if (!GetFileSizeEx(hFile, &fsize)) {
        fwprintf_s(stderr, L"Error %lu in GetFileSizeEx\n", GetLastError());
        goto errexit;
    }

    uint8_t* const buffer = malloc(fsize.QuadPart); // caller is responsible for freeing this buffer.
    if (!buffer) {
        fputws(L"Memory allocation error: malloc returned NULL", stderr);
        goto errexit;
    }

    if (ReadFile(hFile, buffer, fsize.QuadPart, nread_bytes, NULL)) {
        CloseHandle(hFile);
        return buffer;
    } else {
        fwprintf_s(stderr, L"Error %lu in ReadFile\n", GetLastError());
        free(buffer);
        goto errexit;
    }

errexit:
    CloseHandle(hFile);
    return NULL;
}

static inline bool writeFile(
    _In_ const wchar_t* const restrict file_path, _In_ const uint8_t* const restrict buffer, _In_ const size_t size
) {
    const HANDLE const hfile = CreateFileW(file_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfile == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in CreateFileW\n", GetLastError());
        return false;
    }

    DWORD nbyteswritten = 0;
    if (!WriteFile(hfile, buffer, size, &nbyteswritten, NULL)) {
        fwprintf_s(stderr, L"Error %lu in WriteFile\n", GetLastError());
        CloseHandle(hfile);
        return false;
    }

    CloseHandle(hfile);
    return true;
}

#endif // !__FILEIO_H__
