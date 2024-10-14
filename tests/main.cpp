#include <algorithm>
#include <array>
#include <numeric>
#include <random>

#include <gtest/gtest.h>

static constexpr auto  N_RANDNUMS { 200LLU };    // redefinition
static constexpr auto  N_EXTRANDOMS { 5000LLU }; // redefinition
static constexpr float RAND_LLIMIT { -25.0 }, RAND_ULIMIT { 25.0 };

extern std::unique_ptr<float[], std::default_delete<float[]>> randoms_extra;
extern std::unique_ptr<float[], std::default_delete<float[]>> sorted_randoms_extra;

extern std::array<unsigned short, N_RANDNUMS> randoms;
extern std::array<unsigned short, N_RANDNUMS> sorted_randoms;

auto wmain() -> int {
    randoms_extra        = std::make_unique_for_overwrite<float[]>(N_EXTRANDOMS);
    sorted_randoms_extra = std::make_unique_for_overwrite<float[]>(N_EXTRANDOMS);

    assert(randoms_extra.get());
    assert(sorted_randoms_extra.get());

    std::mt19937_64                       rndeng { std::random_device()() };
    std::uniform_real_distribution<float> urdist { RAND_LLIMIT, RAND_ULIMIT };
    std::generate(randoms_extra.get(), randoms_extra.get() + N_EXTRANDOMS, [&rndeng, &urdist]() noexcept -> float {
        return urdist(rndeng);
    });

    std::copy(randoms_extra.get(), randoms_extra.get() + N_EXTRANDOMS, sorted_randoms_extra.get());
    std::sort(
        sorted_randoms_extra.get(), sorted_randoms_extra.get() + N_EXTRANDOMS, std::greater<decltype(sorted_randoms_extra)::element_type> {}
    );

    std::generate(randoms.begin(), randoms.end(), rndeng);
    std::copy(randoms.begin(), randoms.end(), sorted_randoms.begin());
    std::sort(sorted_randoms.begin(), sorted_randoms.end(), std::greater<decltype(sorted_randoms)::value_type> {});

    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
