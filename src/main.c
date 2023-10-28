#include <fileio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined _DEBUG || defined DEBUG
int                           main(void) {
    const uint8_t* content;

#else
int wmain(_In_opt_ int argc, _In_opt_count_(argc) wchar_t* argv[]) {
#endif // DEBUG || _DEBUG

        return EXIT_SUCCESS;
}