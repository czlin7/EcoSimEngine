#include "EcoSimEngine/simulation/SimulationWorld.hpp"

#include "EcoSimEngine/component/ComponentIndices.hpp"
#include "EcoSimEngine/system/AISystem.hpp"
#include "EcoSimEngine/system/MovementSystem.hpp"

SimulationWorld::SimulationWorld()
    : m_entityManager(m_systemManager, m_componentManager)
{
    // Movement system
    m_systemManager.RegisterSystem<MovementSystem>();

    Signature movementSignature;
    movementSignature.set(COMP_INDEX_CTransform);

    m_systemManager.SetSignature<MovementSystem>(movementSignature);

    // AI system
    m_systemManager.RegisterSystem<AISystem>();

    Signature aiSignature;
    aiSignature.set(COMP_INDEX_CBehavior);
    aiSignature.set(COMP_INDEX_CTransform);

    m_systemManager.SetSignature<AISystem>(aiSignature);
}

void SimulationWorld::update(float dt)
{
    m_entityManager.update();

    if (auto ai = m_systemManager.GetSystem<AISystem>())
    {
        ai->update(m_entityManager, m_componentManager, dt);
    }

    if (auto movement = m_systemManager.GetSystem<MovementSystem>())
    {
        movement->update(
            m_entityManager,
            m_componentManager,
            dt);
    }
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
