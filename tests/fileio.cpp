#include <test.hpp>

extern "C" {
#define restrict
#include <fileio.h>
#undef restrict
}

TEST(FILEIO, OPEN) {
    unsigned long size {};

    const auto* buffer = ::___open(LR"(./../media/bronze.jpg)", &size); // an image file
    EXPECT_TRUE(buffer);
    ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
    EXPECT_EQ(size, 589'001LLU);

    buffer = ::___open(LR"(./../media/excerpt.txt)",
                       &size); // a text file
    EXPECT_TRUE(buffer);
    ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
    EXPECT_EQ(size, 999'530LLU);

    buffer = ::___open(LR"(./../media/sqlite3.dll)", &size); // a binary file (DLL)
    EXPECT_TRUE(buffer);
    ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
    EXPECT_EQ(size, 1'541'912LLU);
}

TEST(FILEIO, WRITE) {
    // TODO
}
