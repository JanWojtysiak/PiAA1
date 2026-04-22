#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <limits>
#include <random>

class RandomGenerator {
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;

public:
    RandomGenerator();
    void reset();

    int generate();
};

#endif
