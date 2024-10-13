#include <algorithm>
#include <numeric>
#include <random>

#include <gtest/gtest.h>

std::unique_ptr<float[], std::default_delete<float[]>> randoms_extra;
std::unique_ptr<float[], std::default_delete<float[]>> sorted_randoms_extra;
extern unsigned long long                              N_EXTRANDOMS;
extern float                                           RAND_LLIMIT, RAND_ULIMIT;

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
    std::sort(sorted_randoms_extra.get(), sorted_randoms_extra.get() + N_EXTRANDOMS, std::greater<float> {});

    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
