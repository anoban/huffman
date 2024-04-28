#include <huffman.h>

uint8_t* openFile(_In_ const wchar_t* restrict file_name, _Inout_ size_t* const nread_bytes) {
    *nread_bytes        = 0;
    DWORD        rbytes = 0;
    const HANDLE hFile  = CreateFileW(file_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in CreateFileW while opening %s\n", GetLastError(), file_name);
        return NULL;
    }

    LARGE_INTEGER fsize;
    if (!GetFileSizeEx(hFile, &fsize)) {
        fwprintf_s(stderr, L"Error %lu in GetFileSizeEx while opening %s\n", GetLastError(), file_name);
        goto PREMATURE_RETURN;
    }

    uint8_t* const buffer = malloc(fsize.QuadPart); // caller is responsible for freeing this buffer.
    if (!buffer) {
        fputws(L"Memory allocation error: malloc returned NULL", stderr);
        goto PREMATURE_RETURN;
    }

    if (ReadFile(hFile, buffer, fsize.QuadPart, &rbytes, NULL)) {
        CloseHandle(hFile);
        *nread_bytes = rbytes;
        return buffer;
    } else {
        fwprintf_s(stderr, L"Error %lu in ReadFile while opening %s\n", GetLastError(), file_name);
        free(buffer);
        goto PREMATURE_RETURN;
    }

PREMATURE_RETURN:
    CloseHandle(hFile);
    return NULL;
}

bool writeFile(_In_ const wchar_t* const restrict file_path, _In_ const uint8_t* const restrict buffer, _In_ const size_t size) {
    const HANDLE hfile = CreateFileW(file_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfile == INVALID_HANDLE_VALUE) {
        fwprintf_s(stderr, L"Error %lu in CreateFileW while creating %s\n", GetLastError(), file_path);
        return false;
    }

    DWORD nbyteswritten = 0;
    if (!WriteFile(hfile, buffer, size, &nbyteswritten, NULL)) {
        fwprintf_s(stderr, L"Error %lu in WriteFile while creating %s\n", GetLastError(), file_path);
        CloseHandle(hfile);
        return false;
    }

    CloseHandle(hfile);
    return true;
}
