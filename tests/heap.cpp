#define __VERBOSE_TEST_IO__
#include <algorithm>
#include <array>
#include <memory>

#include <test.hpp>

extern std::unique_ptr<float[], std::default_delete<float[]>> randoms_extra;
extern std::unique_ptr<float[], std::default_delete<float[]>> sorted_randoms_extra;
extern std::array<unsigned short, N_RANDNUMS>                 randoms;
extern std::array<unsigned short, N_RANDNUMS>                 sorted_randoms;

namespace heap {

    struct HeapFixture : public testing::Test {
            huffman::heap_t heap {};

            inline virtual void SetUp() noexcept override { ASSERT_TRUE(huffman::heap_init(&heap, ::comp)); }

            inline virtual void TearDown() noexcept override { huffman::heap_clean(&heap); }
    };

    TEST_F(HeapFixture, INIT) {
        // in google test, EXPECT_XXX family of macros dispactch their call to class template EqHelper where type deduction happens
        // so comp and &comp have different types in the context of template type deduction, hence the use of std::addressof (a simple & will also work)

        EXPECT_FALSE(heap.count);
        EXPECT_EQ(heap.capacity, DEFAULT_HEAP_CAPACITY);
        EXPECT_EQ(heap.predptr, std::addressof(::comp));
        EXPECT_TRUE(heap.tree);
    }

    TEST_F(HeapFixture, PUSH) {
        ::node_pointer ptrs[N_RANDNUMS] { nullptr }; // pointers to heap allocated random numbers

        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            ptrs[i] = reinterpret_cast<::node_pointer>(::malloc(sizeof(::node_type)));
            // ptrs[i] = ptrs_sorted[i] = new (std::nothrow) T; // bad idea because heap_clean will call free() on memory allocated with new
            ASSERT_TRUE(ptrs[i]);
            *ptrs[i] = randoms[i];
        }

        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            EXPECT_TRUE(huffman::heap_push(&heap, ptrs[i]));
            // heap_push() works okay
            EXPECT_EQ(*reinterpret_cast<::node_pointer>(heap.tree[0]), *std::max_element(randoms.cbegin(), randoms.cbegin() + i + 1));
        }

        EXPECT_EQ(heap.count, N_RANDNUMS);
        EXPECT_EQ(heap.capacity, DEFAULT_HEAP_CAPACITY);
        EXPECT_EQ(heap.predptr, std::addressof(::comp));
        EXPECT_TRUE(heap.tree);
    }

    TEST_F(HeapFixture, POP) {
        ::node_pointer ptrs[N_RANDNUMS] { nullptr };

        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            ptrs[i] = reinterpret_cast<::node_pointer>(::malloc(sizeof(::node_type)));
            ASSERT_TRUE(ptrs[i]);
            *ptrs[i] = randoms[i];

            EXPECT_TRUE(huffman::heap_push(&heap, ptrs[i])); // heap_push() works okay
            EXPECT_EQ(*reinterpret_cast<::node_pointer>(heap.tree[0]), *std::max_element(randoms.cbegin(), randoms.cbegin() + i + 1));
        }

        EXPECT_EQ(heap.count, N_RANDNUMS);
        EXPECT_EQ(heap.capacity, DEFAULT_HEAP_CAPACITY);
        EXPECT_EQ(heap.predptr, std::addressof(::comp));
        EXPECT_TRUE(heap.tree);

        ::node_pointer popped {};
        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            EXPECT_TRUE(huffman::heap_pop(&heap, reinterpret_cast<void**>(&popped)));
            EXPECT_EQ(*popped, sorted_randoms[i]);
        }
    }

    namespace stress_test { // pushing the implementation to extremes

        struct tsignal final { // a dummy aggregate type representing a thermal/heat signal
                unsigned  observatory_id;
                float     temperature;
                float     coord_x, coord_y;
                struct tm time;

                constexpr bool operator>(_In_ const tsignal& other) const noexcept { return temperature > other.temperature; }
        };

        using node_type    = tsignal; // shadows the global aliases
        using node_pointer = tsignal*;

        static_assert(sizeof(node_type) == 52);
        static_assert(std::is_standard_layout_v<node_type>);

        // this will test reallocations inside heap_push() and the use of a non primitive type as the stored type in heap
        TEST(HEAP, STRESS_TEST) { // cannot use HeapFixture here because we need a custom compare function
            huffman::heap_t heap {};
            huffman::heap_init(&heap, ::nodecomp<node_type>);

            node_pointer _ptr {};

            for (size_t i = 0; i < N_EXTRANDOMS; ++i) {
                _ptr = reinterpret_cast<node_pointer>(malloc(sizeof(node_type)));
                ASSERT_TRUE(_ptr);
                _ptr->observatory_id = i;
                _ptr->temperature    = randoms_extra[i];

                EXPECT_TRUE(huffman::heap_push(&heap, _ptr)); // expecting reallocations
                EXPECT_EQ(
                    reinterpret_cast<node_pointer>(heap.tree[0])->temperature,
                    *std::max_element(randoms_extra.get(), randoms_extra.get() + i + 1)
                );
            }

            for (size_t i = 0; i < N_EXTRANDOMS; ++i) {
                huffman::heap_pop(&heap, reinterpret_cast<void**>(&_ptr));
                // wprintf_s(L"%zu :: %.5f, %.5f\n", i, _ptr->temperature, sorted_randoms_extra[i]);
                // wprintf_s(L"%5zu :: %.5f\n", i, reinterpret_cast<node_pointer>(heap.tree[0])->temperature);  // !!!!!SEHs ahead
                EXPECT_EQ(_ptr->temperature, sorted_randoms_extra[i]);
            }

            huffman::heap_clean(&heap);
        }

    } // namespace stress_test

} // namespace heap
