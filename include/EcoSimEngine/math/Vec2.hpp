#pragma once

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <SFML/System/Vector2.hpp>

template <typename T>
class Vec2
{
public:
    T x{}, y{};

    constexpr Vec2() = default;

    constexpr Vec2(T x, T y)
        : x(x), y(y) {}

    // constructor from a single value, x and y will be the same
    // explicit usage to avoid accidental conversions
    constexpr explicit Vec2(T value)
        : x(value), y(value) {}

    // allow automatic conversion to sf::Vector2<T>
    // this lets us pass Vec2 to functions that expect sf::Vector2<T>
    constexpr operator sf::Vector2<T>() const noexcept
    {
        return sf::Vector2<T>(x, y);
    }

    // unary negation
    constexpr Vec2 operator-() const noexcept
    {
        return Vec2(-x, -y);
    }

    // Comparison operators

    constexpr bool operator==(const Vec2 &other) const
    {
        return x == other.x && y == other.y;
    }
    constexpr bool operator!=(const Vec2 &other) const
    {
        return !(*this == other);
    }

    // Arithmetic operations

    constexpr Vec2 operator+(const Vec2 &other) const
    {
        return Vec2(x + other.x, y + other.y);
    }
    constexpr Vec2 operator-(const Vec2 &other) const
    {
        return Vec2(x - other.x, y - other.y);
    }
    constexpr Vec2 operator*(T scalar) const
    {
        return Vec2(x * scalar, y * scalar);
    }
    Vec2 operator/(T scalar) const
    {
        if (scalar == 0)
        {
            throw std::domain_error("Division by zero in Vec2 division");
        }
        return Vec2(x / scalar, y / scalar);
    }

    // Compound assignment operators

    constexpr Vec2 &operator+=(const Vec2 &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    constexpr Vec2 &operator-=(const Vec2 &other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    constexpr Vec2 &operator*=(T scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    Vec2 &operator/=(T scalar)
    {
        if (scalar == 0)
        {
            throw std::domain_error("Division by zero in Vec2 division");
        }
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Length and normalization

    double distSq(const Vec2 &other) const
    {
        double dx = static_cast<double>(x) - static_cast<double>(other.x);
        double dy = static_cast<double>(y) - static_cast<double>(other.y);
        return dx * dx + dy * dy;
    }

    double dist(const Vec2 &other) const
    {
        return std::sqrt(distSq(other));
    }

    double length() const
    {
        return std::hypot(
            static_cast<double>(x),
            static_cast<double>(y));
    }

    Vec2<double> normalized() const
    {
        const double len = length();

        if (len == 0.0)
        {
            throw std::domain_error("Cannot normalize a zero-length vector");
        }

        return Vec2<double>(
            static_cast<double>(x) / len,
            static_cast<double>(y) / len);
    }

    Vec2 abs() const
    {
        return Vec2(std::abs(x), std::abs(y));
    }

    // Dot product
    double dot(const Vec2 &other) const
    {
        return static_cast<double>(x) * other.x + static_cast<double>(y) * other.y;
    }

    // 2d scalar cross product
    double cross(const Vec2 &other) const
    {
        return static_cast<double>(x) * other.y - static_cast<double>(y) * other.x;
    }

    // Angle between two vectors in radians
    double angleBetween(const Vec2 &other) const
    {
        double dotProd = dot(other);
        const double lengths = length() * other.length();
        if (lengths == 0)
            throw std::domain_error("Zero length vector in angleBetween calculation");

        // Clamp the value to the range [-1, 1] to avoid NaN due to floating point precision issues
        double cosTheta = std::clamp(dotProd / lengths, -1.0, 1.0);
        return std::acos(cosTheta);
    }

    // Rotation by an angle in radians around the origin
    Vec2<double> rotated(double radians) const noexcept
    {
        const double cosTheta = std::cos(radians);
        const double sinTheta = std::sin(radians);

        const double dx = static_cast<double>(x);
        const double dy = static_cast<double>(y);

        return Vec2<double>(
            dx * cosTheta - dy * sinTheta,
            dx * sinTheta + dy * cosTheta);
    }

    // Rotation by an angle in radians around a specific point
    Vec2<double> rotatedAround(
        double radians,
        const Vec2 &point) const noexcept
    {
        const double cosTheta = std::cos(radians);
        const double sinTheta = std::sin(radians);

        const double pointX = static_cast<double>(point.x);
        const double pointY = static_cast<double>(point.y);

        const double translatedX =
            static_cast<double>(x) - pointX;

        const double translatedY =
            static_cast<double>(y) - pointY;

        return Vec2<double>(
            translatedX * cosTheta - translatedY * sinTheta + pointX,
            translatedX * sinTheta + translatedY * cosTheta + pointY);
    }

    // Static utility functions

    // Create from angle in radians
    static Vec2<double> fromAngle(double radians) noexcept
    {
        return Vec2<double>(
            std::cos(radians),
            std::sin(radians));
    }

    // Create from angle in radians with a specific magnitude
    static Vec2<double> fromAngle(
        double radians,
        double magnitude) noexcept
    {
        return Vec2<double>(
            std::cos(radians) * magnitude,
            std::sin(radians) * magnitude);
    }
};

// Left side arithmetic operations
template <typename T>
inline Vec2<T> operator*(T scalar, const Vec2<T> &vec) noexcept
{
    return Vec2<T>(vec.x * scalar, vec.y * scalar);
}

// Type aliases for convenience

using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;