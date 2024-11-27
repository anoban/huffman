#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include <test.hpp>

extern "C" {
#define restrict
#include <pqueue.h>
#undef restrict
}

extern "C" {
    // return true when a swap is needed, i.e when the child is heavier than the parent
    [[nodiscard]] static __declspec(noinline) bool __stdcall comp(_In_ const void* const child, _In_ const void* const parent) noexcept {
        return *reinterpret_cast<pqueue_test::constant_node_pointer>(child) > *reinterpret_cast<pqueue_test::constant_node_pointer>(parent);
    }

    // argument typedef int (__cdecl* _CoreCrtSecureSearchSortCompareFunction)(void*, void const*, void const*)
    [[nodiscard, deprecated]] static __declspec(noinline) int __cdecl ptrcomp( // to be used with qsort_s()
        _In_opt_ [[maybe_unused]] void* const context,                         // we do not need this for our tests
        _In_ pqueue_test::constant_node_pointer* const current,                // cannot use long double here directly
        _In_ pqueue_test::constant_node_pointer* const next
    ) noexcept {
        assert(reinterpret_cast<uintptr_t>(*current) & reinterpret_cast<uintptr_t>(*next));
        return (**current == **next) ? 0 : (**current > **next) ? 1 : -1;
    }
}
template<typename _TyNode>
[[nodiscard]] static __declspec(noinline) bool __stdcall nodecomp(_In_ const void* const child, _In_ const void* const parent) noexcept
    requires requires(const _TyNode& _left, const _TyNode& _right) {
        // explicitly calling the .operator>() member instead of using > because we do not want primitive types meeting this type constraint
        _left.operator>(_right);
    } {
    return (reinterpret_cast<typename std::add_pointer_t<typename std::add_const_t<_TyNode>>>(child))
        ->operator>(*reinterpret_cast<typename std::add_pointer_t<typename std::add_const_t<_TyNode>>>(parent));
}

extern std::vector<pqueue_test::node_type> randoms;        // defined in main.cpp
extern std::vector<pqueue_test::node_type> randoms_sorted; // defined in main.cpp

extern std::vector<pqueue_stress_test::node_type> stress_test_randoms;        // defined in main.cpp
extern std::vector<pqueue_stress_test::node_type> stress_test_randoms_sorted; // defined in main.cpp

struct PQueueFixture : public testing::Test {
        ::pqueue prqueue {};

        inline virtual void SetUp() noexcept override { ASSERT_TRUE(::pqueue_init(&prqueue, ::comp)); }

        inline virtual void TearDown() noexcept override { ::pqueue_clean(&prqueue); }
};

// FOLLOWING ARE TESTS THAT DO NOT INVOLVE ANY REALLOCATION INSIDE THE PRIORITY QUEUE

TEST_F(PQueueFixture, INIT) {
    EXPECT_FALSE(prqueue.count);
    EXPECT_EQ(prqueue.capacity, DEFAULT_PQUEUE_CAPACITY);
    EXPECT_EQ(prqueue.predptr, std::addressof(::comp));
    EXPECT_TRUE(prqueue.tree);
}

TEST_F(PQueueFixture, PUSH) {
    pqueue_test::node_pointer temp {}; // pointers to heap allocated random numbers

    for (size_t i = 0; i < ELEMENT_COUNT_WITHOUT_REALLOCATION; ++i) {
        temp = reinterpret_cast<pqueue_test::node_pointer>(::malloc(sizeof(pqueue_test::node_type)));
        EXPECT_TRUE(temp);
        *temp = randoms[i];
        EXPECT_TRUE(::pqueue_push(&prqueue, temp));
    }

    EXPECT_EQ(prqueue.count, ELEMENT_COUNT_WITHOUT_REALLOCATION);
    EXPECT_EQ(prqueue.capacity, DEFAULT_PQUEUE_CAPACITY);
    EXPECT_EQ(prqueue.predptr, std::addressof(::comp));
    EXPECT_TRUE(prqueue.tree);

    EXPECT_EQ(*reinterpret_cast<pqueue_test::node_pointer>(prqueue.tree[0]), randoms_sorted[0]);
}

TEST_F(PQueueFixture, POP) {
    pqueue_test::node_pointer temp {};

    for (size_t i = 0; i < ELEMENT_COUNT_WITHOUT_REALLOCATION; ++i) {
        temp = reinterpret_cast<pqueue_test::node_pointer>(::malloc(sizeof(pqueue_test::node_type)));
        EXPECT_TRUE(temp);
        *temp = randoms[i];
        EXPECT_TRUE(::pqueue_push(&prqueue, temp));
    }

    EXPECT_EQ(prqueue.count, ELEMENT_COUNT_WITHOUT_REALLOCATION);
    EXPECT_EQ(prqueue.capacity, DEFAULT_PQUEUE_CAPACITY);
    EXPECT_EQ(prqueue.predptr, std::addressof(::comp));
    EXPECT_TRUE(prqueue.tree);

    pqueue_test::node_pointer popped {};
    for (size_t i = 0; i < ELEMENT_COUNT_WITHOUT_REALLOCATION; ++i) {
        EXPECT_TRUE(::pqueue_pop(&prqueue, reinterpret_cast<void**>(&popped)));
        EXPECT_EQ(*popped, randoms_sorted[i]);
    }

    // once pqueue_pop() has given up the last node, it will call pqueue_clean() so ....
    EXPECT_FALSE(prqueue.count);
    EXPECT_FALSE(prqueue.capacity);
    EXPECT_FALSE(prqueue.predptr);
    EXPECT_FALSE(prqueue.tree);
}

TEST_F(PQueueFixture, PEEK) {
    pqueue_test::node_pointer temp {};

    for (size_t i = 0; i < ELEMENT_COUNT_WITHOUT_REALLOCATION; ++i) {
        temp = reinterpret_cast<pqueue_test::node_pointer>(::malloc(sizeof(pqueue_test::node_type)));
        EXPECT_TRUE(temp);
        *temp = randoms[i];
        EXPECT_TRUE(::pqueue_push(&prqueue, temp));
    }

    EXPECT_EQ(prqueue.count, ELEMENT_COUNT_WITHOUT_REALLOCATION);
    EXPECT_EQ(prqueue.capacity, DEFAULT_PQUEUE_CAPACITY);
    EXPECT_EQ(prqueue.predptr, std::addressof(::comp));
    EXPECT_TRUE(prqueue.tree);

    pqueue_test::node_pointer popped {}, peek {};
    for (size_t i = 0; i < ELEMENT_COUNT_WITHOUT_REALLOCATION; ++i) {
        peek = reinterpret_cast<pqueue_test::node_pointer>(::pqueue_peek(&prqueue));
        EXPECT_EQ(*peek, randoms_sorted[i]);
        EXPECT_TRUE(::pqueue_pop(&prqueue, reinterpret_cast<void**>(&popped)));
    }

    EXPECT_FALSE(prqueue.count);
    EXPECT_FALSE(prqueue.capacity);
    EXPECT_FALSE(prqueue.predptr);
    EXPECT_FALSE(prqueue.tree);
}

TEST(PQUEUE, STRESS_TEST) {
    ::pqueue prqueue {};
    ::pqueue_init(&prqueue, ::nodecomp<pqueue_stress_test::node_type>);

    EXPECT_FALSE(prqueue.count);
    EXPECT_EQ(prqueue.capacity, DEFAULT_PQUEUE_CAPACITY);
    EXPECT_EQ(prqueue.predptr, std::addressof(::nodecomp<pqueue_stress_test::node_type>));
    EXPECT_TRUE(prqueue.tree);

    pqueue_stress_test::node_pointer nodeptr {};

    for (unsigned i = 0; i < ELEMENT_COUNT_WITH_REALLOCATION; ++i) {
        nodeptr = reinterpret_cast<pqueue_stress_test::node_pointer>(::malloc(sizeof(pqueue_stress_test::node_type)));
        ASSERT_TRUE(nodeptr);
        *nodeptr = stress_test_randoms[i];
        EXPECT_TRUE(::pqueue_push(&prqueue, nodeptr));
    }

    EXPECT_EQ(prqueue.count, ELEMENT_COUNT_WITH_REALLOCATION);
    EXPECT_EQ(prqueue.capacity, ELEMENT_COUNT_WITH_REALLOCATION); // take into account the 5 extra allocations
    EXPECT_EQ(prqueue.predptr, std::addressof(::nodecomp<pqueue_stress_test::node_type>));
    EXPECT_TRUE(prqueue.tree);

    for (size_t i = 0; i < ELEMENT_COUNT_WITH_REALLOCATION; ++i) {
        EXPECT_TRUE(::pqueue_pop(&prqueue, reinterpret_cast<void**>(&nodeptr)));
        EXPECT_EQ(nodeptr->unit_price, stress_test_randoms_sorted[i].unit_price);
    }

    EXPECT_FALSE(prqueue.count);
    EXPECT_FALSE(prqueue.capacity);
    EXPECT_FALSE(prqueue.predptr);
    EXPECT_FALSE(prqueue.tree);
}
