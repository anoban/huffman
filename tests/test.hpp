#pragma once
#define __VERBOSE_TEST_IO__

#include <gtest/gtest.h>

// the problem with namspacing <huffman.h> prior to including <gtest/gtest.h> is that all the symbols from headers directly and
// indirecty included in <huffman.h> get scoped inside the namespace, won't be available in the global namespace this includes symbols from __STDC__ headers :(
// but the header guards copied into our TUs will prevent these headers from being reincluded in gtest.h, hence we run in to a slew of errors
// hence, avoiding this approach

static constexpr unsigned long long ELEMENT_COUNT_WITHOUT_REALLOCATION { 5 << 7 };
static constexpr unsigned long long ELEMENT_COUNT_WITH_REALLOCATION { 5 << 10 };

namespace pqueue_test {

    using node_type             = unsigned long long; // for testing using fixtures
    using node_pointer          = node_type*;
    using constant_node_pointer = const node_type*;

} // namespace pqueue_test

namespace pqueue_stress_test {

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

    static_assert(sizeof(pqueue_stress_test::node_type) == 24);
    static_assert(std::is_standard_layout_v<pqueue_stress_test::node_type>);

} // namespace pqueue_stress_test