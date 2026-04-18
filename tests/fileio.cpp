#include <algorithm>
#include <random>

#include <test.hpp>

extern "C" {
#define restrict
#include <fileio.h>
#undef restrict
}

TEST(fileio, __open) {
    long size {};

    const auto* buffer = ::__read(R"(./files/bronze.jpg)", &size); // an image file
    EXPECT_TRUE(buffer);
    ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
    EXPECT_EQ(size, 589'001);

    buffer = ::__read(R"(./files/mobydick.txt)", &size); // a text file
    EXPECT_TRUE(buffer);
    ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
    EXPECT_EQ(size, 988'635);

    buffer = ::__read(R"(./files/table.csv)", &size); // a csv file
    EXPECT_TRUE(buffer);
    ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
    EXPECT_EQ(size, 1'113'978);

    buffer = ::__read(R"(./files/sqlite3.dll)", &size); // a binary file (DLL)
    EXPECT_TRUE(buffer);
    ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
    EXPECT_EQ(size, 1'541'912);
}

TEST(fileio, __write) {
    unsigned long long         buffsize { 1'000'000 };
    std::vector<unsigned char> buffer(buffsize);
    std::mt19937_64            rndengine { std::random_device {}() };
    long                       fsize {};
    unsigned char*             fbuffer {};

    std::generate(buffer.begin(), buffer.end(), [&rndengine]() noexcept -> auto {
        return static_cast<unsigned char>(rndengine() % std::numeric_limits<unsigned char>::max());
    });
    EXPECT_TRUE(::__write(R"(./files/temp01.dat)", buffer.data(), buffsize));
    fbuffer = ::__read(R"(./files/temp01.dat)", &fsize);
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
    EXPECT_TRUE(::__write(R"(./files/temp02.dat)", buffer.data(), buffsize));
    fbuffer = ::__read(R"(./files/temp02.dat)", &fsize);
    EXPECT_TRUE(fbuffer);
    EXPECT_TRUE(std::equal(buffer.cbegin(), buffer.cend(), fbuffer));
    EXPECT_EQ(fsize, buffsize);
    ::free(fbuffer);

    EXPECT_FALSE(::remove(R"(./files/temp01.dat)")); // ::remove() returns 0 upon successful deletion
    EXPECT_FALSE(::remove(R"(./files/temp02.dat)"));
}
