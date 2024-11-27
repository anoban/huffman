#include <huffman.h>

static unsigned long long frequencies[256] = { 0 };

int wmain(_In_opt_ [[maybe_unused]] int argc, _In_opt_count_(argc) [[maybe_unused]] wchar_t* argv[]) {
    unsigned                   fsize = 0;
    const unsigned char* const fbuff = open(L"./media/synth.bin", &fsize);
    assert(fbuff);

    scan_frequencies(fbuff, fsize, frequencies); // populate the frequency table
    free(fbuff);

    for (unsigned i = 0; i < 256; ++i) wprintf_s(L"%4u   %10llu\n", i, frequencies[i]);

    return EXIT_SUCCESS;
}
