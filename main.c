#include <stdint.h>

#include "huffman.h"

int32_t wmain(_In_opt_ const int32_t argc, _In_opt_count_(argc) const wchar_t* argv[]) {
    uint64_t       nbytes = 0;
    const uint8_t* bytes  = open(L"./JaneEyre.txt", &nbytes);

    if (!bytes) {
        wprintf_s(stderr, L"Error: (Line %d, File %s) open returned NULL.\n", __LINE__, __FILEW__);
        return EXIT_FAILURE;
    }

    huffman_t hftree;
    init_huffmantree(&hftree);
    init_freqtable(&hftree, bytes, nbytes);

    free(bytes);
    return EXIT_SUCCESS;
}
