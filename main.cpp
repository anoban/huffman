#include "huffman.hpp"

int wmain(_In_opt_ int32_t argc, _In_opt_count_(argc) wchar_t* argv[]) {
    uint64_t nbytes {};
    auto     buffer { utilities::open(argv[1], &nbytes) };
    auto     hftree { huffman::huffman_t(buffer) };
    hftree.build();

    return EXIT_SUCCESS;
}