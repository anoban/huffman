#include <huffman.h>

static unsigned long long frequencies[BYTECOUNT]                                   = { 0 };
static btnode_t           global_btnode_buffer[GLOBAL_BTNODE_BUFFER_FIXEDCAPACITY] = { 0 }; // for use with pqueues

int wmain(_In_opt_ [[maybe_unused]] int argc, _In_opt_count_(argc) [[maybe_unused]] wchar_t* argv[]) {
    unsigned                   filesize   = 0;
    const unsigned char* const filebuffer = open(L"./media/therepublic.txt", &filesize);
    assert(filebuffer);

    scan_frequencies(filebuffer, filesize, frequencies); // populate the frequency table
    free(filebuffer);

    bntree_t huffman = build_huffman_tree(frequencies, global_btnode_buffer);

    for (unsigned i = 0; i < 256; ++i) wprintf_s(L"%4u (%c)  %10llu\n", i, i, frequencies[i]);

    return EXIT_SUCCESS;
}
