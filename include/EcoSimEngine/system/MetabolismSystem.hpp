#pragma once

#include <algorithm>
#include <vector>

#include "EcoSimEngine/ecs/EntityManager.hpp"
#include "EcoSimEngine/system/System.hpp"

class MetabolismSystem : public System
{
public:
    static constexpr float kStarvationDamagePerSecond{10.0f}; // Damage applied to entities that are starving

    void update(
        EntityManager &em,
        ComponentManager &cm,
        float dt)
    {
        if (dt <= 0.0f)
            return;

        std::vector<EntityId> entitiesToDestroy;

        for (EntityId id : mEntities)
        {
            auto entity = em.getEntityById(id);

            if (!entity || !entity->isActive())
            {
                continue;
            }

            if (!cm.has<CEnergy>(id) || !cm.has<CHealth>(id))
            {
                continue;
            }

            auto &energy = cm.get<CEnergy>(id);
            auto &health = cm.get<CHealth>(id);

            if (energy.current > 0.0f)
            {
                energy.current = std::max(
                    0.0f,
                    energy.current - energy.consumptionRate * dt);

                continue;
            }

            health.health = std::max(
                0.0f,
                health.health - kStarvationDamagePerSecond * dt);

            if (health.health <= 0.0f)
            {
                entitiesToDestroy.push_back(id);
            }
        }

        for (EntityId id : entitiesToDestroy)
        {
            auto entity = em.getEntityById(id);

            if (entity)
            {
                em.destroyEntity(entity);
            }
        }
    }
};