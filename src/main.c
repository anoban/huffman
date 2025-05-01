#define __TEST__
#define __VERBOSE_TEST_IO__

#include <huffman.h>

static unsigned long long frequencies[BYTECOUNT]                            = { 0 }; // 2KiBs
static btnode_t           pqueue_buffer[GLOBAL_BTNODE_BUFFER_FIXEDCAPACITY] = { 0 }; // 32KiBs - for use with priority queues
static btnode_t           bntree_buffer[GLOBAL_BTNODE_BUFFER_FIXEDCAPACITY] = { 0 }; // 32KiBs - for use with binary trees

// the whole buffer being just 32KiBs, can probably fit in the CPU's caches :))

int wmain(_In_opt_ [[maybe_unused]] int argc, _In_opt_count_(argc) [[maybe_unused]] wchar_t* argv[]) {
    unsigned                   filesize   = 0;
    const unsigned char* const filebuffer = open(L"./media/sqlite3.dll", &filesize);
    assert(filebuffer);

    scan_frequencies(filebuffer, filesize, frequencies); // populate the frequency table
    free(filebuffer);

    bntree_t huffman = build_huffman_tree(frequencies, pqueue_buffer, bntree_buffer);

    // for (unsigned i = 0; i < 256; ++i) wprintf_s(L"%4u (%c)  %10llu\n", i, i, frequencies[i]);

    return EXIT_SUCCESS;
}
