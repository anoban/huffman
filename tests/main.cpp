#include <algorithm>
#include <array>
#include <memory>
#include <random>

#include <test.hpp>

// all these globals are used in the .cpp files for tests
std::vector<pqueue_test::node_type> randoms;
std::vector<pqueue_test::node_type> randoms_sorted;

std::vector<pqueue_stress_test::node_type> stress_test_randoms;
std::vector<pqueue_stress_test::node_type> stress_test_randoms_sorted;

auto main() -> int {
    randoms.resize(ELEMENT_COUNT_WITHOUT_REALLOCATION);
    randoms_sorted.resize(ELEMENT_COUNT_WITHOUT_REALLOCATION);

    stress_test_randoms.resize(ELEMENT_COUNT_WITH_REALLOCATION);
    stress_test_randoms_sorted.resize(ELEMENT_COUNT_WITH_REALLOCATION);

    std::mt19937_64 rndeng { std::random_device {}() };

    std::generate(randoms.begin(), randoms.end(), [&rndeng]() noexcept -> auto { return rndeng(); });
    std::copy(randoms.cbegin(), randoms.cend(), randoms_sorted.begin());
    std::sort(randoms_sorted.begin(), randoms_sorted.end(), std::greater<pqueue_test::node_type> {});

    std::for_each(stress_test_randoms.begin(), stress_test_randoms.end(), [&rndeng](pqueue_stress_test::node_type& elem) noexcept -> void {
        elem.unit_price = rndeng() / 100.00;
    });
    std::copy(stress_test_randoms.cbegin(), stress_test_randoms.cend(), stress_test_randoms_sorted.begin());
    std::sort(stress_test_randoms_sorted.begin(), stress_test_randoms_sorted.end(), std::greater<pqueue_stress_test::node_type> {});

    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
