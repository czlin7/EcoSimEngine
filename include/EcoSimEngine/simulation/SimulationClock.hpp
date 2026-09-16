#pragma once

#include <cstdint>

class SimulationClock
{
    static constexpr double EPSILON = 1e-12;

    double m_fixedStep{1.0 / 60.0};
    double m_accumulator{0.0};
    double m_speed{1.0};

    std::uint64_t m_tick{0};
    std::uint64_t m_maxTicks{100000};

public:
    SimulationClock() = default;

    SimulationClock(
        double fixedStep,
        std::uint64_t maxTicks);

    void configure(
        double fixedStep,
        std::uint64_t maxTicks);

    void reset() noexcept;

    void addElapsed(double realDeltaSeconds);

    [[nodiscard]] bool canStep() const noexcept;
    void consumeStep();

    void setSpeed(double speed);

    [[nodiscard]] double fixedStep() const noexcept;
    [[nodiscard]] double speed() const noexcept;

    [[nodiscard]] std::uint64_t tick() const noexcept;
    [[nodiscard]] std::uint64_t maxTicks() const noexcept;

    [[nodiscard]] bool finished() const noexcept;
};