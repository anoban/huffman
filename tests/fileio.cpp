#include <random>

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

    buffer = ::___open(LR"(./../media/mobydick.txt)", &size); // a text file
    EXPECT_TRUE(buffer);
    ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
    EXPECT_EQ(size, 988'635LLU);

    buffer = ::___open(LR"(./../media/therepublic.txt)", &size); // another text file
    EXPECT_TRUE(buffer);
    ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
    EXPECT_EQ(size, 1'219'217LLU);

    buffer = ::___open(LR"(./../media/sqlite3.dll)", &size); // a binary file (DLL)
    EXPECT_TRUE(buffer);
    ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
    EXPECT_EQ(size, 1'541'912LLU);
}

TEST(FILEIO, WRITE) {
    unsigned long long         buffsize { 1'000'000 };
    std::vector<unsigned char> buffer(buffsize);
    std::mt19937_64            rndengine { std::random_device {}() };
    unsigned long              fsize {};
    unsigned char*             fbuffer {};

    std::generate(buffer.begin(), buffer.end(), [&rndengine]() noexcept -> auto {
        return static_cast<unsigned char>(rndengine() % std::numeric_limits<unsigned char>::max());
    });
    EXPECT_TRUE(::___write(LR"(./../media/temp01.dat)", buffer.data(), buffsize));
    fbuffer = ::___open(LR"(./../media/temp01.dat)", &fsize);
    EXPECT_TRUE(fbuffer);
    EXPECT_TRUE(std::equal(buffer.cbegin(), buffer.cend(), fbuffer));
    EXPECT_EQ(fsize, buffsize);
    ::free(fbuffer);

    fbuffer   = nullptr;
    buffsize *= 2;
    buffer.resize(buffsize);
    fsize = 0;

    std::generate(buffer.begin(), buffer.end(), [&rndengine]() noexcept -> auto {
        return static_cast<unsigned char>(rndengine() % std::numeric_limits<unsigned char>::max());
    });
    EXPECT_TRUE(::___write(LR"(./../media/temp02.dat)", buffer.data(), buffsize));
    fbuffer = ::___open(LR"(./../media/temp02.dat)", &fsize);
    EXPECT_TRUE(fbuffer);
    EXPECT_TRUE(std::equal(buffer.cbegin(), buffer.cend(), fbuffer));
    EXPECT_EQ(fsize, buffsize);
    ::free(fbuffer);

    EXPECT_TRUE(::DeleteFileW(LR"(./../media/temp01.dat)"));
    EXPECT_TRUE(::DeleteFileW(LR"(./../media/temp02.dat)"));
}
