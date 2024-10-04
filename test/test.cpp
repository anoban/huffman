#include <cstdlib>
#include <cstring>
#include <ctime>

#include <pch.hpp>

using node_type             = unsigned; // for testing
using constant_node_pointer = const node_type*;

// return true when a swap is needed, i.e when the child is heavier than the parent
static __declspec(noinline) bool __stdcall comp(_In_ const void* const child, _In_ const void* const parent) noexcept {
    return *reinterpret_cast<constant_node_pointer>(child) > *reinterpret_cast<constant_node_pointer>(parent);
}

// argument typedef int (__cdecl* _CoreCrtSecureSearchSortCompareFunction)(void*, void const*, void const*)
static __declspec(noinline) int __cdecl ptrcompare( // to be used with qsort_s()
    _In_opt_ [[maybe_unused]] void* const context,  // we do not need this for our tests
    _In_ constant_node_pointer* const     current,  // cannot use long double here directly
    _In_ constant_node_pointer* const     next
) noexcept {
    assert(reinterpret_cast<uintptr_t>(*current) & reinterpret_cast<uintptr_t>(*next));
    return (**current == **next) ? 0 : (**current > **next) ? 1 : -1;
}

namespace bitops {

    TEST(BITOPS, GETBIT) {
        for (size_t i = 0; i < BITSTREAM_BIT_COUNT; ++i)
            EXPECT_EQ(huffman::getbit(bitstream, i), (binstr[i] - 48)); // '0' is 48 and '1' is 49
    }

    TEST(BITOPS, SETBIT) {
        for (size_t i = 0; i < BITSTREAM_BIT_COUNT; ++i) huffman::setbit(mutablestream, i, i % 2);
        // set all odd bits true and even bits false, which will effectively make every byte in the stream equivalent to 0b0101'0101
        for (size_t i = 0; i < BITSTREAM_BYTE_COUNT; ++i) EXPECT_EQ(mutablestream[i], 0b0101'0101);
    }

    TEST(BITOPS, XORBIT) {
        ::memset(mutablestream, 0U, BITSTREAM_BYTE_COUNT); // cleanu up after the previous use
        for (size_t i = 0; i < BITSTREAM_BIT_COUNT; ++i) huffman::xorbit(bitstream, xorbitstream, mutablestream, i);
        for (size_t i = 0; i < BITSTREAM_BIT_COUNT; ++i)
            EXPECT_EQ(huffman::getbit(mutablestream, i), huffman::getbit(bitstream, i) == huffman::getbit(xorbitstream, i) ? false : true);
    }

} // namespace bitops

namespace fileio {

    TEST(FILEIO, OPEN) {
        unsigned long size {};

        const auto* buffer = huffman::___open(LR"(./../media/bronze.jpg)", &size); // an image file
        EXPECT_TRUE(buffer);
        ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
        EXPECT_EQ(size, 589'001LLU);

        buffer = huffman::___open(LR"(./../media/excerpt.txt)",
                                  &size); // a text file
        EXPECT_TRUE(buffer);
        ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
        EXPECT_EQ(size, 999'530LLU);

        buffer = huffman::___open(LR"(./../media/sqlite3.dll)", &size); // a binary file
        EXPECT_TRUE(buffer);
        ::free(reinterpret_cast<void*>(const_cast<unsigned char*>(buffer)));
        EXPECT_EQ(size, 1'541'912LLU);
    }

    TEST(FILEIO, WRITE) {
        // TODO
    }

} // namespace fileio

namespace position {

    TEST(POSITION, PARENTPOSITION) {
        EXPECT_EQ(huffman::parent_position(0), 0LLU);
        EXPECT_EQ(huffman::parent_position(1), 0LLU);
        EXPECT_EQ(huffman::parent_position(2), 0LLU);
        EXPECT_EQ(huffman::parent_position(3), 1LLU);
        EXPECT_EQ(huffman::parent_position(4), 1LLU);
        EXPECT_EQ(huffman::parent_position(5), 2LLU);
        EXPECT_EQ(huffman::parent_position(9), 4LLU);
        EXPECT_EQ(huffman::parent_position(13), 6LLU);
        EXPECT_EQ(huffman::parent_position(20), 9LLU);
        EXPECT_EQ(huffman::parent_position(58), 28LLU);
    }

    TEST(POSITION, LEFTCHILDPOSITION) {
        EXPECT_EQ(huffman::lchild_position(0), 1LLU);
        EXPECT_EQ(huffman::lchild_position(1), 3LLU);
        EXPECT_EQ(huffman::lchild_position(2), 5LLU);
        EXPECT_EQ(huffman::lchild_position(3), 7LLU);
        EXPECT_EQ(huffman::lchild_position(4), 9LLU);
        EXPECT_EQ(huffman::lchild_position(5), 11LLU);
        EXPECT_EQ(huffman::lchild_position(9), 19LLU);
        EXPECT_EQ(huffman::lchild_position(13), 27LLU);
        EXPECT_EQ(huffman::lchild_position(20), 41LLU);
        EXPECT_EQ(huffman::lchild_position(58), 117LLU);
    }

    TEST(POSITION, RIGHTCHILDPOSITION) {
        EXPECT_EQ(huffman::rchild_position(0), 2LLU);
        EXPECT_EQ(huffman::rchild_position(1), 4LLU);
        EXPECT_EQ(huffman::rchild_position(2), 6LLU);
        EXPECT_EQ(huffman::rchild_position(3), 8LLU);
        EXPECT_EQ(huffman::rchild_position(4), 10LLU);
        EXPECT_EQ(huffman::rchild_position(5), 12LLU);
        EXPECT_EQ(huffman::rchild_position(9), 20LLU);
        EXPECT_EQ(huffman::rchild_position(13), 28LLU);
        EXPECT_EQ(huffman::rchild_position(20), 42LLU);
        EXPECT_EQ(huffman::rchild_position(58), 118LLU);
    }

} // namespace position

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
        unsigned* ptrs[N_RANDNUMS] { nullptr };        // pointers to heap allocated random numbers
        unsigned* ptrs_sorted[N_RANDNUMS] { nullptr }; // ptrs sorted in ascending order based on the values they hold

        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            ptrs[i] = ptrs_sorted[i] = reinterpret_cast<unsigned*>(::malloc(sizeof(unsigned)));
            // ptrs[i] = ptrs_sorted[i] = new (std::nothrow) T; // bad idea because heap_clean will call free() on memory allocated with new
            ASSERT_TRUE(ptrs[i]);
            *ptrs[i] = randoms[i];
        }

        // sort the ptrs_sorted array in descending order
        std::sort(std::begin(ptrs_sorted), std::end(ptrs_sorted), [](const auto* const current, const auto* const next) noexcept -> bool {
            return *current > *next;
        });

        // qsort_s(ptrs_sorted, N_RANDNUMS, sizeof(T*), ptrcompare, nullptr); // since std::sort is touted to be more efficient
        // for (size_t i = 0; i < N_RANDNUMS; ++i) wprintf_s(L"%u\n", *ptrs_sorted[i]);

        for (size_t i = 0; i < N_RANDNUMS; ++i) EXPECT_TRUE(huffman::heap_push(&heap, ptrs[i]));

        EXPECT_EQ(heap.count, N_RANDNUMS);
        EXPECT_EQ(heap.capacity, DEFAULT_HEAP_CAPACITY);
        EXPECT_EQ(heap.predptr, std::addressof(::comp));
        EXPECT_TRUE(heap.tree);

        // if all pushes worked as expected, the node at offset 0 must contain the largest value
        EXPECT_EQ(*reinterpret_cast<unsigned*>(heap.tree[0]), sorted_randoms[0]);
    }

    TEST_F(HeapFixture, POP) {
        unsigned* ptrs[N_RANDNUMS] { nullptr };

        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            ptrs[i] = reinterpret_cast<unsigned*>(::malloc(sizeof(unsigned)));
            ASSERT_TRUE(ptrs[i]);
            *ptrs[i] = randoms[i];
            EXPECT_TRUE(huffman::heap_push(&heap, ptrs[i]));
        }

        EXPECT_EQ(heap.count, N_RANDNUMS);
        EXPECT_EQ(heap.capacity, DEFAULT_HEAP_CAPACITY);
        EXPECT_EQ(heap.predptr, std::addressof(::comp));
        EXPECT_TRUE(heap.tree);

        unsigned* popped {};
        for (size_t i = 0; i < N_RANDNUMS; ++i) {
            EXPECT_TRUE(huffman::heap_pop(&heap, reinterpret_cast<void**>(&popped)));
            EXPECT_EQ(*popped, sorted_randoms[i]);
        }
    }

    struct node final {
            unsigned           id;
            unsigned long long frequency;

            constexpr bool operator>(const node& other) const noexcept { return frequency > other.frequency; }
    };

    static __declspec(noinline) bool __stdcall nodecomp(_In_ const void* const child, _In_ const void* const parent) noexcept {
        return *reinterpret_cast<const node*>(child) > *reinterpret_cast<const node*>(parent);
    }

    // this will test reallocations inside the heap and the use of a non primitive type as the stored element
    TEST(HEAP, EXTENDED) { // cannot use test fixtures using HeapFixture here because we need a custom compare function
        huffman::heap_t heap {};
        huffman::heap_init(&heap, nodecomp);

        node* _temp {};

        for (size_t i = 0; i < 2048; ++i) { // expecting 5 reallocations
            _temp = reinterpret_cast<node*>(malloc(sizeof(node)));
            ASSERT_TRUE(_temp);
            _temp->id        = i;
            _temp->frequency = randoms_ext[i];

            EXPECT_TRUE(huffman::heap_push(&heap, _temp));
        }

        for (size_t i = 0; i < N_EXTRANDOMS; ++i) {
            huffman::heap_pop(&heap, reinterpret_cast<void**>(&_temp));
            // EXPECT_EQ(_temp->frequency, sorted_randoms_ext[i]);
        }

        huffman::heap_clean(&heap);
    }

} // namespace heap

namespace pqueue {

    struct QueueFixture : public testing::Test {
            huffman::PQueue pqueue {};

            inline virtual void SetUp() noexcept override { ASSERT_TRUE(huffman::PQueueInit(&pqueue, ::comp)); }

            inline virtual void TearDown() noexcept override { huffman::PQueueClean(&pqueue); }
    };

    TEST_F(QueueFixture, INIT) {
        //
    }
} // namespace pqueue