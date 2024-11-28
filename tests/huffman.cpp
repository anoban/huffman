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
        pqueue_t prqueue;

        inline void virtual SetUp() noexcept override {
            std::knuth_b randeng { static_cast<unsigned>(::time(nullptr)) };
            prqueue = pqueue_init(btnode_buffer.data(), PQUEUE_MAX_ELEMENT_COUNT);
            std::generate(dummy_filebuffer.begin(), dummy_filebuffer.end(), [&randeng]() noexcept -> auto {
                return randeng() % (std::numeric_limits<unsigned char>::max() / 2); // deliberately skewing the distribution of bytes
            });
        }

        inline void virtual TearDown() noexcept override { pqueue_clean(&prqueue); }
};