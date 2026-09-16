#pragma once

#include <cstdint>
#include <random>

#include "EcoSimEngine/component/ComponentManager.hpp"
#include "EcoSimEngine/ecs/EntityManager.hpp"
#include "EcoSimEngine/system/SystemManager.hpp"

class SimulationWorld
{
private:
    SystemManager m_systemManager;
    ComponentManager m_componentManager;
    EntityManager m_entityManager;

    std::mt19937 m_rng{std::mt19937::default_seed};

public:
    SimulationWorld();

    void update(float dt);

    void reseed(std::uint32_t seed);

    [[nodiscard]] std::mt19937& rng() noexcept;

    [[nodiscard]] EntityManager& entityManager() noexcept;
    [[nodiscard]] ComponentManager& componentManager() noexcept;
    [[nodiscard]] SystemManager& systemManager() noexcept;
};
