#pragma once

#ifndef __HUFFMAN__
    #define __HUFFMAN__

    #include <array>
    #include <cstdint>
    #include <stdexcept>
    #include <string>
    #include <vector>

    #ifdef _WIN32
        #define _AMD64_
        #define WIN32_LEAN_AND_MEAN
        #define WIN32_EXTRA_MEAN
    #endif

    #include <errhandlingapi.h>
    #include <fileapi.h>
    #include <handleapi.h>

namespace utilities {
    static inline std::vector<uint8_t> open(_In_ const std::wstring&& fpath, _Inout_ uint64_t* nread_bytes) noexcept {
        *nread_bytes = 0;
        HANDLE               handle {};
        std::vector<uint8_t> buffer {};

        handle = CreateFileW(fpath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr);

        if (handle != INVALID_HANDLE_VALUE) {
            LARGE_INTEGER fsize {};
            if (!GetFileSizeEx(handle, &fsize)) {
                fwprintf_s(stderr, L"Error %lu in GetFileSizeEx (Line %d, File %s)\n", GetLastError(), __LINE__, __FILEW__);
                return buffer;
            }

            buffer.resize(fsize.QuadPart);
            if (!buffer.empty()) {
                if (ReadFile(handle, buffer.data(), fsize.QuadPart, reinterpret_cast<LPDWORD>(nread_bytes), nullptr)) {
                    CloseHandle(handle);
                    return buffer;
                } else {
                    fwprintf_s(stderr, L"Error %lu in ReadFile\n", GetLastError());
                }
            } else {
                fputws(L"Memory allocation error: vector resizing failed", stderr);
            }
        } else {
            fwprintf_s(stderr, L"Error %lu in CreateFileW\n", GetLastError());
        }

        CloseHandle(handle);
        buffer.clear();
        return buffer;
    }

} // namespace utilities

namespace huffman { } // namespace huffman

#endif                // !__HUFFMAN__