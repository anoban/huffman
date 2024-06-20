#include <huffman.h>

uint8_t* __cdecl open(_In_ const wchar_t* restrict filepath, _Inout_ unsigned long* const nbytes) {
    *nbytes              = 0;
    const HANDLE64 hFile = CreateFileW(filepath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in CreateFileW while opening %s\n", GetLastError(), filepath);
        return NULL;
    }

    LARGE_INTEGER fsize = { .QuadPart = 0 };
    if (!GetFileSizeEx(hFile, &fsize)) {
        fwprintf_s(stderr, L"Error %lu in GetFileSizeEx while opening %s\n", GetLastError(), filepath);
        goto PREMATURE_RETURN;
    }

    uint8_t* const buffer = malloc(fsize.QuadPart); // caller is responsible for freeing this buffer.
    if (!buffer) {
        fputws(L"Memory allocation error: malloc returned NULL", stderr);
        goto PREMATURE_RETURN;
    }

    if (!ReadFile(hFile, buffer, fsize.QuadPart, nbytes, NULL)) {
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

bool __cdecl write(_In_ const wchar_t* const restrict filepath, _In_ const uint8_t* const restrict buffer, _In_ const unsigned long size) {
    const HANDLE hfile = CreateFileW(filepath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfile == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in CreateFileW while creating %s\n", GetLastError(), filepath);
        return false;
    }

    DWORD nbyteswritten = 0;
    if (!WriteFile(hfile, buffer, size, &nbyteswritten, NULL)) {
        fwprintf_s(stderr, L"Error %lu in WriteFile while creating %s\n", GetLastError(), filepath);
        CloseHandle(hfile);
        return false;
    }

    CloseHandle(hfile);
    return true;
}
