#include <random>
#include <vector>

#include <test.hpp>
extern "C" {
#define restrict
#include <custom.h>
#undef restrict
}

static constexpr size_t _PQUEUE_MAX_ELEMENT_COUNT { 8 << 11 };

struct PQueueFixture : public testing::Test {
        pqueue_t              prqueue {};
        std::vector<bntree_t> buffer;

        inline void virtual SetUp() noexcept override {
            buffer.resize(_PQUEUE_MAX_ELEMENT_COUNT);
            pqueue_init(&prqueue, buffer.data(), _PQUEUE_MAX_ELEMENT_COUNT);
        }

        inline void virtual TearDown() noexcept override { pqueue_clean(&prqueue); }
};