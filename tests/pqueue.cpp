#include <algorithm>
#include <array>
#include <memory>

#include <test.hpp>
extern "C" {
#define restrict
#include <pqueue.h>
#undef restrict
}

extern std::vector<float>                     randoms_extra;
extern std::vector<float>                     sorted_randoms_extra;
extern std::array<unsigned short, N_RANDNUMS> randoms;
extern std::array<unsigned short, N_RANDNUMS> sorted_randoms;

namespace priority_queue {

    struct PQueueFixture : public testing::Test {
            ::pqueue prqueue {};

            inline virtual void SetUp() noexcept override { ASSERT_TRUE(::pqueue_init(&prqueue, ::comp)); }

            inline virtual void TearDown() noexcept override { ::pqueue_clean(&prqueue); }
    };

    TEST_F(PQueueFixture, INIT) {
        EXPECT_FALSE(prqueue.count);
        EXPECT_EQ(prqueue.capacity, DEFAULT_PQUEUE_CAPACITY);
        EXPECT_EQ(prqueue.predptr, std::addressof(::comp));
        EXPECT_TRUE(prqueue.tree);
    }

    TEST_F(PQueueFixture, PUSH) {
        ::node_pointer ptrs[N_RANDNUMS] { nullptr }; // pointers to heap allocated random numbers

        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            ptrs[i] = reinterpret_cast<::node_pointer>(::malloc(sizeof(::node_type)));
            ASSERT_TRUE(ptrs[i]);
            *ptrs[i] = randoms[i];
        }

        for (size_t i = 0; i < N_RANDNUMS; ++i) EXPECT_TRUE(::pqueue_push(&prqueue, ptrs[i]));

        EXPECT_EQ(prqueue.count, N_RANDNUMS);
        EXPECT_EQ(prqueue.capacity, DEFAULT_PQUEUE_CAPACITY);
        EXPECT_EQ(prqueue.predptr, std::addressof(::comp));
        EXPECT_TRUE(prqueue.tree);

        EXPECT_EQ(*reinterpret_cast<::node_pointer>(prqueue.tree[0]), sorted_randoms[0]);
    }

    TEST_F(PQueueFixture, POP) {
        ::node_pointer ptrs[N_RANDNUMS] { nullptr };

        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            ptrs[i] = reinterpret_cast<::node_pointer>(::malloc(sizeof(::node_type)));
            ASSERT_TRUE(ptrs[i]);
            *ptrs[i] = randoms[i];
            EXPECT_TRUE(::pqueue_push(&prqueue, ptrs[i]));
        }

        EXPECT_EQ(prqueue.count, N_RANDNUMS);
        EXPECT_EQ(prqueue.capacity, DEFAULT_PQUEUE_CAPACITY);
        EXPECT_EQ(prqueue.predptr, std::addressof(::comp));
        EXPECT_TRUE(prqueue.tree);

        ::node_pointer popped {};
        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            EXPECT_TRUE(::pqueue_pop(&prqueue, reinterpret_cast<void**>(&popped)));
            EXPECT_EQ(*popped, sorted_randoms[i]);
        }
    }

    namespace stress_test {

        struct record final {
                unsigned id;  // record id
                unsigned yor; // year of release
                float    unit_price;
                unsigned _padding;
                double   sales;

                constexpr bool operator>(_In_ const record& other) const noexcept { return unit_price > other.unit_price; }
        };

        using node_type    = record;
        using node_pointer = record*;

        static_assert(sizeof(stress_test::node_type) == 24, "");
        static_assert(std::is_standard_layout_v<stress_test::node_type>, "");

        TEST(PQUEUE, STRESS_TEST) {
            ::pqueue prqueue {};
            ::pqueue_init(&prqueue, ::nodecomp<stress_test::node_type>);

            stress_test::node_pointer _ptr {};

            for (unsigned i = 0; i < N_EXTRANDOMS; ++i) {
                _ptr = reinterpret_cast<stress_test::node_pointer>(malloc(sizeof(stress_test::node_type)));
                ASSERT_TRUE(_ptr);
                _ptr->id         = i;
                _ptr->unit_price = randoms_extra[i];

                EXPECT_TRUE(::pqueue_push(&prqueue, _ptr));
            }

            for (size_t i = 0; i < N_EXTRANDOMS; ++i) {
                ::pqueue_pop(&prqueue, reinterpret_cast<void**>(&_ptr));
                EXPECT_EQ(_ptr->unit_price, sorted_randoms_extra[i]);
            }

            ::pqueue_clean(&prqueue);
        }

    } // namespace stress_test

} // namespace priority_queue
