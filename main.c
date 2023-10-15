#include <stdint.h>

#include "huffman.h"

int32_t wmain(_In_opt_ const int32_t argc, _In_opt_count_(argc) const wchar_t* argv[]) {
    if (argc != 2) {
        fputws(L"Error: main.exe accepts/takes only one argument [file name]\n", stderr);
        return EXIT_FAILURE;
    }

    uint64_t       nbytes = 0;
    const uint8_t* bytes  = open(argv[1], &nbytes);

    if (!bytes) {
        fwprintf_s(stderr, L"Error: (Line %d, File %s) open returned NULL.\n", __LINE__, __FILEW__);
        return EXIT_FAILURE;
    }

    huffman_t hftree;
    init_huffmantree(&hftree);
    init_freqtable(&hftree, bytes, nbytes);
    sort_bytes(&hftree, predicate);
    init_leaves(&hftree);
    printbytes(&hftree);

    wprintf_s(L"%s has %llu different bytes in it.\n", argv[1], hftree.nleaves);

    free(bytes);
    return EXIT_SUCCESS;
}
