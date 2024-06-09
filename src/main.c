#ifndef __TEST__

    #include <huffman.h>

int wmain(_In_opt_ int argc, _In_opt_count_(argc) wchar_t* argv[]) {
    if (argc == 1) {
        fputws(L"Error :: Programme invoked without any arguments!\nUsage :: huffman.exe <PATH_TO_FILE>\n", stderr);
        return EXIT_FAILURE;
    }

    size_t               fsize   = 0;
    const uint8_t* const content = openFile(argv[1], &fsize);

    wprintf_s(L"File size: %zu bytes.\n", fsize);
    free(content);
    return EXIT_SUCCESS;
}

#endif // !__TEST__