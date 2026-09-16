#pragma once

#include <cmath>
#include <random>

#include "EcoSimEngine/math/Vec2.hpp"

inline float randomFloat(
    std::mt19937 &rng,
    float min,
    float max)
{
    std::uniform_real_distribution<float> distribution(min, max);

    return distribution(rng);
}

inline Vec2f randomUnitVector(std::mt19937& rng)
{
    const float angle = randomFloat(
        rng,
        0.0f,
        2.0f * 3.14159265358979323846f);

    return Vec2f{ std::cos(angle), std::sin(angle) };
}
