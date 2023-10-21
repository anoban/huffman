#pragma once

#ifndef __HUFFMAN__
    #define __HUFFMAN__

    #include <array>
    #include <concepts>
    #include <cstdint>
    #include <queue>
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

namespace huffman {

    struct node_t {
            uint64_t freq {};
            node_t*  left {};
            node_t*  right {};

            node_t(_In_ node_t* l, _In_ node_t* r) noexcept {
                freq  = l->freq + r->freq; // aggregate frequency of the two child nodes.
                left  = l;
                right = r;
            }

            node_t() = default;
    };

    struct leaf_t : public node_t {
            uint8_t byte {};

            template<typename T> requires std::integral<T> leaf_t(_In_ const uint8_t b, _In_ const T f) {
                byte = b;
                freq = f;
                left = right = nullptr;
            }
    };

    constexpr bool   operator<(_In_ const node_t& lhs, _In_ const node_t& rhs) { return lhs.freq < rhs.freq; }

    constexpr size_t MAX_TOTAL_NODES { 511 };

    struct huffman_t {
            std::array<node_t, MAX_TOTAL_NODES>       nodes {};   // an array of 511 node_t s.
            std::array<uint64_t, 256>                 freqs {};   // an array holding the frequencies of each byte in the file.
            std::vector<std::pair<uint8_t, uint64_t>> bytes {};   // bytes sorted by ascending frequency.
            uint64_t                                  nleaves {}; // number of leaf nodes in the tree.
            uint64_t                                  nlinks {};  // number of link nodes in the tree.
            bool                                      isfreqtable_initialized {};
            bool                                      is_initialized {};

            huffman_t(_In_ const std::vector<uint8_t>& stream) {
                for (auto& e : stream) freqs.at(e)++;
                for (size_t i = 0; i < 256; ++i)
                    if (freqs.at(i)) {
                        bytes.push_back(std::pair<uint8_t, uint64_t> { static_cast<uint8_t>(i), freqs.at(i) });
                        nleaves++;
                    }
                std::sort(
                    bytes.begin(),
                    bytes.end(),
                    [](_In_ const std::pair<uint8_t, uint64_t>& __this, _In_ const std::pair<uint8_t, uint64_t>& __next) {
                        return __this.second < __next.second;
                    }
                );
                for (auto& p : bytes) wprintf_s(L"%3d: %10llu\n", p.first, p.second);
            }

            void build() noexcept {
                std::priority_queue<node_t> pq {};

                for (auto& p : bytes) pq.push(leaf_t { p.first, p.second });
            }
    };

} // namespace huffman

#endif // !__HUFFMAN__