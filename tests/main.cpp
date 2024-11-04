#include <algorithm>
#include <array>
#include <memory>
#include <random>

#include <test.hpp>

// all these globals are used in the cpp files for tests
std::vector<float>                     randoms_extra;
std::vector<float>                     sorted_randoms_extra;
std::array<unsigned short, N_RANDNUMS> randoms;
std::array<unsigned short, N_RANDNUMS> sorted_randoms;

auto main() -> int {
    randoms_extra.resize(N_EXTRANDOMS);
    sorted_randoms_extra.resize(N_EXTRANDOMS);

    std::mt19937_64                       rndeng { std::random_device {}() };
    std::uniform_real_distribution<float> urdist { RAND_LLIMIT, RAND_ULIMIT };

    std::generate(randoms_extra.begin(), randoms_extra.end(), [&rndeng, &urdist]() noexcept -> float { return urdist(rndeng); });
    std::copy(randoms_extra.cbegin(), randoms_extra.cend(), sorted_randoms_extra.begin());
    std::sort(sorted_randoms_extra.begin(), sorted_randoms_extra.end(), std::greater<decltype(sorted_randoms_extra)::value_type> {});

    std::generate(randoms.begin(), randoms.end(), [&rndeng]() noexcept -> auto {
        return static_cast<decltype(randoms)::value_type>(rndeng());
    });
    std::copy(randoms.begin(), randoms.end(), sorted_randoms.begin());
    std::sort(sorted_randoms.begin(), sorted_randoms.end(), std::greater<decltype(sorted_randoms)::value_type> {});

    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
