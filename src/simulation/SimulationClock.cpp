#include "EcoSimEngine/simulation/SimulationClock.hpp"

#include <cmath>
#include <stdexcept>

SimulationClock::SimulationClock(
    double fixedStep,
    std::uint64_t maxTicks)
{
    configure(fixedStep, maxTicks);
}

void SimulationClock::configure(
    double fixedStep,
    std::uint64_t maxTicks)
{
    if (!std::isfinite(fixedStep) || fixedStep <= 0.0)
    {
        throw std::invalid_argument(
            "SimulationClock fixed step must be a positive finite value");
    }

    m_fixedStep = fixedStep;
    m_maxTicks = maxTicks;

    reset();
}

void SimulationClock::reset() noexcept
{
    m_accumulator = 0.0;
    m_tick = 0;
}

void SimulationClock::addElapsed(double realDeltaSeconds)
{
    if (!std::isfinite(realDeltaSeconds) || realDeltaSeconds < 0.0)
    {
        throw std::invalid_argument(
            "SimulationClock elapsed time must be a non-negative finite value");
    }

    m_accumulator += realDeltaSeconds * m_speed;
}

bool SimulationClock::canStep() const noexcept
{
    if (finished())
    {
        return false;
    }

    return m_accumulator + EPSILON >= m_fixedStep;
}

void SimulationClock::consumeStep()
{
    if (!canStep())
    {
        throw std::logic_error(
            "SimulationClock cannot consume a step when no step is available");
    }

    m_accumulator -= m_fixedStep;

    if (m_accumulator < 0.0)
    {
        m_accumulator = 0.0;
    }

    ++m_tick;
}

void SimulationClock::setSpeed(double speed)
{
    if (!std::isfinite(speed) || speed <= 0.0)
    {
        throw std::invalid_argument(
            "SimulationClock speed must be a positive finite value");
    }

    m_speed = speed;
}

double SimulationClock::fixedStep() const noexcept
{
    return m_fixedStep;
}

double SimulationClock::speed() const noexcept
{
    return m_speed;
}

std::uint64_t SimulationClock::tick() const noexcept
{
    return m_tick;
}

std::uint64_t SimulationClock::maxTicks() const noexcept
{
    return m_maxTicks;
}

bool SimulationClock::finished() const noexcept
{
    return m_tick >= m_maxTicks;
}