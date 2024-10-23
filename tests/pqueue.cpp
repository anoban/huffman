#include <algorithm>
#include <array>
#include <memory>

#include <test.hpp>

extern std::unique_ptr<float[], std::default_delete<float[]>> randoms_extra;
extern std::unique_ptr<float[], std::default_delete<float[]>> sorted_randoms_extra;
extern std::array<unsigned short, N_RANDNUMS>                 randoms;
extern std::array<unsigned short, N_RANDNUMS>                 sorted_randoms;

namespace pqueue {

    struct PQueueFixture : public testing::Test {
            huffman::PQueue pqueue {};

            inline virtual void SetUp() noexcept override { ASSERT_TRUE(huffman::PQueueInit(&pqueue, ::comp)); }

            inline virtual void TearDown() noexcept override { huffman::PQueueClean(&pqueue); }
    };

    TEST_F(PQueueFixture, INIT) {
        EXPECT_FALSE(pqueue.count);
        EXPECT_EQ(pqueue.capacity, DEFAULT_PQUEUE_CAPACITY);
        EXPECT_EQ(pqueue.predptr, std::addressof(::comp));
        EXPECT_TRUE(pqueue.tree);
    }

    TEST_F(PQueueFixture, PUSH) {
        ::node_pointer ptrs[N_RANDNUMS] { nullptr }; // pointers to heap allocated random numbers

        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            ptrs[i] = reinterpret_cast<::node_pointer>(::malloc(sizeof(::node_type)));
            ASSERT_TRUE(ptrs[i]);
            *ptrs[i] = randoms[i];
        }

        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            EXPECT_TRUE(huffman::PQueuePush(&pqueue, ptrs[i]));
            EXPECT_EQ(*reinterpret_cast<::node_pointer>(pqueue.tree[0]), *std::max_element(randoms.cbegin(), randoms.cbegin() + i + 1));
        }

        EXPECT_EQ(pqueue.count, N_RANDNUMS);
        EXPECT_EQ(pqueue.capacity, DEFAULT_PQUEUE_CAPACITY);
        EXPECT_EQ(pqueue.predptr, std::addressof(::comp));
        EXPECT_TRUE(pqueue.tree);

        EXPECT_EQ(*reinterpret_cast<::node_pointer>(pqueue.tree[0]), sorted_randoms[0]);
    }

    TEST_F(PQueueFixture, POP) {
        ::node_pointer ptrs[N_RANDNUMS] { nullptr };

        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            ptrs[i] = reinterpret_cast<::node_pointer>(::malloc(sizeof(::node_type)));
            ASSERT_TRUE(ptrs[i]);
            *ptrs[i] = randoms[i];
            EXPECT_TRUE(huffman::PQueuePush(&pqueue, ptrs[i]));
        }

        EXPECT_EQ(pqueue.count, N_RANDNUMS);
        EXPECT_EQ(pqueue.capacity, DEFAULT_PQUEUE_CAPACITY);
        EXPECT_EQ(pqueue.predptr, std::addressof(::comp));
        EXPECT_TRUE(pqueue.tree);

        ::node_pointer popped {};
        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            EXPECT_TRUE(huffman::PQueuePop(&pqueue, reinterpret_cast<void**>(&popped)));
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

        static_assert(sizeof(stress_test::node_type) == 24);
        static_assert(std::is_standard_layout_v<stress_test::node_type>);

        TEST(PQUEUE, STRESS_TEST) {
            huffman::PQueue pqueue {};
            huffman::PQueueInit(&pqueue, ::nodecomp<stress_test::node_type>);

            stress_test::node_pointer _ptr {};

            for (size_t i = 0; i < N_EXTRANDOMS; ++i) {
                _ptr = reinterpret_cast<stress_test::node_pointer>(malloc(sizeof(stress_test::node_type)));
                ASSERT_TRUE(_ptr);
                _ptr->id         = i;
                _ptr->unit_price = randoms_extra[i];

                EXPECT_TRUE(huffman::PQueuePush(&pqueue, _ptr));
                EXPECT_EQ(
                    reinterpret_cast<node_pointer>(pqueue.tree[0])->unit_price,
                    *std::max_element(randoms_extra.get(), randoms_extra.get() + i + 1)
                );
            }

            for (size_t i = 0; i < N_EXTRANDOMS; ++i) {
                huffman::PQueuePop(&pqueue, reinterpret_cast<void**>(&_ptr));
                EXPECT_EQ(_ptr->unit_price, sorted_randoms_extra[i]);
            }

            huffman::PQueueClean(&pqueue);
        }

    } // namespace stress_test

} // namespace pqueue
