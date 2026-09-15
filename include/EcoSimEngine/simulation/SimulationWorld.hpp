#pragma once

#include "EcoSimEngine/component/ComponentManager.hpp"
#include "EcoSimEngine/ecs/EntityManager.hpp"
#include "EcoSimEngine/system/SystemManager.hpp"

class SimulationWorld
{
private:
    SystemManager m_systemManager;
    ComponentManager m_componentManager;
    EntityManager m_entityManager;

public:
    SimulationWorld();

    void update(float dt);

    EntityManager& entityManager() noexcept;
    ComponentManager& componentManager() noexcept;
    SystemManager& systemManager() noexcept;
};
