#include <huffman.h>

// the problem with the data structures in the headers is that they liberally rely on type erasure for the sake of supporting arbitraty user defined types
// we lose a lot of type safety and performance because this requires storing type erased heap allocated pointers instead of directly storing the values
// as this would make these data structures tightly coupled to a single type, hence losing their "library" status

// since this is not important to us and we need maximum performance, we'll make variants tailor made for huffman algorithms
// embedding associated user defined types directly inside the data structures, leveraging the tight coupling to eliminate
// type erasure, gratuitous heap allocations and maximize stack usage

int main(_In_opt_ int argc, _In_opt_count_(argc) char* argv[]) {
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    // if (argc == 1) {
    //     fputws(L"Error :: programme invoked without any arguments!\nUsage :: huffman.exe <path>\n", stderr);
    //     return EXIT_FAILURE;
    // }

    // TODO

    return EXIT_SUCCESS;
}
