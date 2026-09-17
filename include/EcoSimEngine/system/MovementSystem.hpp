#pragma once

#include "EcoSimEngine/system/System.hpp"
#include "EcoSimEngine/component/ComponentIndices.hpp"
#include "EcoSimEngine/ecs/Entity.hpp"
#include "EcoSimEngine/ecs/EntityManager.hpp"


class MovementSystem : public System {
public:
    static Signature requiredSignature() {
        Signature signature;
        signature.set(COMP_INDEX_CTransform);
        return signature;
    }

    // EntityManager must implement getEntityById(size_t)
    void update(EntityManager& em, ComponentManager& cm, float dt)
    {
        if (dt <= 0.0f) return; // avoid division by zero

        for (EntityId id : mEntities) {
            auto e = em.getEntityById(id);
            if (!e || !e->isActive()) continue;
            if (!cm.has<CTransform>(id)) continue; // has position and velocity

            auto& t = cm.get<CTransform>(id);

            // clamp speed using behavior if present
            float maxSpeed = 10.0f;
            if (cm.has<CBehavior>(id)) maxSpeed = cm.get<CBehavior>(id).maxSpeed;

            float sp = t.velocity.length();
            if (sp > maxSpeed) t.velocity = t.velocity * (maxSpeed / sp);

            // integrate
            t.pos += t.velocity * dt;

            // damping
            t.velocity = t.velocity * 0.98f;
        }
    }
};
