#include <ctime>
#include <random>
#include <vector>

#include <test.hpp>

extern "C" {
#define restrict
#include <huffman.h>
#undef restrict
}

extern std::vector<unsigned char> dummy_filebuffer; // defined in main.cpp
extern std::vector<::btnode_t>    btnode_buffer;    // defined in main.cpp

struct CustomPQueueFixture : public testing::Test {
        pqueue_t prqueue {};

        inline void virtual SetUp() noexcept override {
            // using a std::vector's internal buffer is okay here because the cognate pqueue_clean() will not try to free this buffer
            // so the destructor can do its job :)
            prqueue = pqueue_init(btnode_buffer.data(), PQUEUE_MAX_ELEMENT_COUNT);
        }

        inline void virtual TearDown() noexcept override { pqueue_clean(&prqueue); }
};

TEST_F(CustomPQueueFixture, INIT) {
    EXPECT_FALSE(prqueue.count);
    EXPECT_EQ(prqueue.capacity, PQUEUE_MAX_ELEMENT_COUNT);
    EXPECT_EQ(prqueue.tree, btnode_buffer.data());
}

TEST_F(CustomPQueueFixture, PUSH) {
    std::knuth_b randeng { static_cast<unsigned>(::time(nullptr)) };
    // hoping for a skewed distribution of bytes
    std::generate(dummy_filebuffer.begin(), dummy_filebuffer.end(), [&randeng]() noexcept -> auto {
        return randeng() % (std::numeric_limits<unsigned char>::max());
    });
}

TEST_F(CustomPQueueFixture, POP) { }

TEST_F(CustomPQueueFixture, PEEK) { }
