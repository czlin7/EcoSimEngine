#pragma once

#include <cmath>

#include <SFML/Graphics/Color.hpp>

inline sf::Color hslToRgb(float h, float s, float l) {
    float c = (1.0f - std::fabs(2.0f * l - 1.0f)) * s;
    float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    const float m = l - c / 2.0f;
     float r = 0.0f, g = 0.0f, b = 0.0f;

    if (h < 60.0f) { r = c; g = x; }
    else if (h < 120.0f) { r = x; g = c; }
    else if (h < 180.0f) { g = c; b = x; }
    else if (h < 240.0f) { g = x; b = c; }
    else if (h < 300.0f) { r = x; b = c; }
    else { r = c; b = x; }

    return sf::Color(
        static_cast<std::uint8_t>((r + m) * 255.0f),
        static_cast<std::uint8_t>((g + m) * 255.0f),
        static_cast<std::uint8_t>((b + m) * 255.0f)
    );
}

