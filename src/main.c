#include <fileio.h>

#if defined _DEBUG || defined DEBUG
int                           main(void) {
    size_t         fsize   = 0;
    const uint8_t* content = openFile(L"./media/anne.png", &fsize);

#else
int wmain(_In_opt_ int argc, _In_opt_count_(argc) wchar_t* argv[]) {
    size_t         fsize   = 0;
    const uint8_t* content = open(argv[1], &fsize);
#endif // DEBUG || _DEBUG

    wprintf_s(L"File size: %zu bytes.\n", fsize);
    free(content);
    return EXIT_SUCCESS;
}