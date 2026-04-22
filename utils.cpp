#include "utils.hpp"

#include <array>
#include <chrono>

RandomGenerator::RandomGenerator()
    : dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max()) {
    reset();
}

void RandomGenerator::reset() {
    std::random_device rd;
    const auto now = static_cast<std::uint64_t>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::array<std::uint32_t, 8> seed_data{
        rd(), rd(), rd(), rd(),
        static_cast<std::uint32_t>(now),
        static_cast<std::uint32_t>(now >> 32),
        rd(), rd()
    };
    std::seed_seq seq(seed_data.begin(), seed_data.end());
    rng.seed(seq);
}

int RandomGenerator::generate() {
    return dist(rng);
}
