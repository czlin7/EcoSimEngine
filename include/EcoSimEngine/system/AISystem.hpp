#pragma once

#include <random>

#include "EcoSimEngine/ecs/Entity.hpp"
#include "EcoSimEngine/ecs/EntityManager.hpp"
#include "EcoSimEngine/math/Vec2.hpp"
#include "EcoSimEngine/system/System.hpp"
#include "EcoSimEngine/utils/Random.hpp"

class AISystem : public System
{
public:
    void update(
        EntityManager &em,
        ComponentManager &cm, 
        float dt, 
        std::mt19937 &rng)
    {
        for (EntityId id : mEntities)
        {
            auto e = em.getEntityById(id);
            if (!e || !e->isActive())
                continue;
            if (!cm.has<CBehavior>(id) || !cm.has<CTransform>(id))
                continue;

            auto &behavior = cm.get<CBehavior>(id);
            auto &transform = cm.get<CTransform>(id);

            behavior.stateTimer -= dt;
            if (behavior.stateTimer <= 0.0f)
            {
                behavior.stateTimer = randomFloat(rng, 1.0f, 5.0f);
                behavior.current = BehaviorState::Wander;
            }

            if (behavior.current == BehaviorState::Wander && transform.velocity.length() < 0.1f)
            {
                const Vec2f direction = randomUnitVector(rng);
                transform.velocity = direction * behavior.movementSpeed;
            }
        }
    }
};
