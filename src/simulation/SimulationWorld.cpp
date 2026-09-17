#include "EcoSimEngine/simulation/SimulationWorld.hpp"

#include "EcoSimEngine/system/AISystem.hpp"
#include "EcoSimEngine/system/MovementSystem.hpp"
#include "EcoSimEngine/system/MetabolismSystem.hpp"

SimulationWorld::SimulationWorld()
    : m_entityManager(m_systemManager, m_componentManager)
{    
    // Register systems
    m_systemManager.RegisterSystem<MovementSystem>();
    m_systemManager.RegisterSystem<AISystem>();
    m_systemManager.RegisterSystem<MetabolismSystem>();
}

void SimulationWorld::update(float dt)
{
    m_entityManager.update();

    if (auto metabolism = m_systemManager.GetSystem<MetabolismSystem>())
    {
        metabolism->update(
            m_entityManager,
            m_componentManager,
            dt);
    }

    if (auto ai = m_systemManager.GetSystem<AISystem>())
    {
        ai->update(m_entityManager, m_componentManager, dt, m_rng);
    }

    if (auto movement = m_systemManager.GetSystem<MovementSystem>())
    {
        movement->update(
            m_entityManager,
            m_componentManager,
            dt);
    }
}

void SimulationWorld::reseed(std::uint32_t seed)
{
    m_rng.seed(seed);
}

std::mt19937& SimulationWorld::rng() noexcept
{
    return m_rng;
}

// getters for the managers
EntityManager& SimulationWorld::entityManager() noexcept
{
    return m_entityManager;
}

ComponentManager& SimulationWorld::componentManager() noexcept
{
    return m_componentManager;
}

SystemManager& SimulationWorld::systemManager() noexcept
{
    return m_systemManager;
}
