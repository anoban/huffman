#ifndef __TEST__
    #include <huffman.h>

int wmain(_In_opt_ int argc, _In_opt_count_(argc) wchar_t* argv[]) {
    if (argc == 1) {
        fputws(L"Error :: programme invoked without any arguments!\nUsage :: huffman.exe <path>\n", stderr);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif // !__TEST__