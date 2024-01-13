#include <huffman.h>
#ifdef _WIN32
    #define __STDC_WANT_SECURE_LIB__ 1  // MSVC headers, for some fucking reason use a plain #if directive for __STDC_WANT_SECURE_LIB__
                                        // not an #ifdef directive, so we need to provide a valued definition for __STDC_WANT_SECURE_LIB__
                                        // a plain #define will result in compiler error!
#endif // _WIN32
#include <limits.h>
#include <bitops.h>
#include <pque.h>

int64_t compareFrequency() {};