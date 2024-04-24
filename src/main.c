#include <fileio.h>

#if defined _DEBUG || defined DEBUG

int         wmain(void) {
    size_t         fsize   = 0;
    const uint8_t* content = openFile(L"./media/anne.png", &fsize);

#else

int wmain(_In_opt_ int argc, _In_opt_count_(argc) wchar_t* argv[]) {
    if (argc == 1) {
        fputws(L"Error :: Programme invoked without any arguments!\nUsage :: huffman.exe <PATH_TO_FILE>\n", stderr);
        return EXIT_FAILURE;
    }

    size_t               fsize   = 0;
    const uint8_t* const content = openFile(argv[1], &fsize);

#endif // DEBUG || _DEBUG

    wprintf_s(L"File size: %zu bytes.\n", fsize);
    free(content);
    return EXIT_SUCCESS;
}