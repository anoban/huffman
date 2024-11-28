#include <huffman.h>

static unsigned long long frequencies[256]                                         = { 0 };
static btnode_t           global_btnode_buffer[GLOBAL_BTNODE_BUFFER_FIXEDCAPACITY] = { 0 }; // for use with pqueues

int wmain(_In_opt_ [[maybe_unused]] int argc, _In_opt_count_(argc) [[maybe_unused]] wchar_t* argv[]) {
    unsigned                   fsize = 0;
    const unsigned char* const fbuff = open(L"./media/therepublic.txt", &fsize);
    assert(fbuff);

    scan_frequencies(fbuff, fsize, frequencies); // populate the frequency table
    free(fbuff);

    pqueue_t prqueue = pqueue_init(global_btnode_buffer, __crt_countof(global_btnode_buffer));

    for (unsigned i = 0; i < 256; ++i) wprintf_s(L"%4u (%c)  %10llu\n", i, i, frequencies[i]);

    return EXIT_SUCCESS;
}
