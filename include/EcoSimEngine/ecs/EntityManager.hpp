#pragma once

#include <map>
#include <memory>
#include <vector>

#include "EcoSimEngine/ecs/Entity.hpp"
#include "EcoSimEngine/system/SystemManager.hpp"
#include "EcoSimEngine/component/ComponentManager.hpp"


using EntityVec = std::vector<std::shared_ptr<Entity>>;
using EntityMap = std::map<std::string, EntityVec>;

class SystemManager; // forward declaration

/**
 * @brief EntityManager class for managing game entities.
 *
 * Responsibilities:
 * - Store all active entities and those pending addition.
 * - Index entities by tag for efficient retrieval.
 * - Remove inactive (destroyed) entities during updates.
 *
 * Entities are managed via std::shared_ptr to allow multiple references
 * without ownership issues. Updates should be called once per simulation tick.
 */
class EntityManager {
    EntityVec m_entities;       // all entities
    EntityVec m_entitiesToAdd;  // entities to add next update
    EntityMap m_entityMap;      // map from entity tag to vectors
    size_t m_totalEntities{ 0 }; // total entities created

	// Pointer to the engine's SystemManger so we can notify on signature change / destruction
    SystemManager* m_systemManager{ nullptr };
    ComponentManager* m_componentManager{ nullptr };


    /**
     * @brief Removes all inactive entities from the given vector.
     *
     * This helper function ensures that only active entities remain in the container.
     * Uses std::erase_if (C++20) to remove elements where the entity's isActive() returns false.
     * 
     * @param vec Reference to a vector of shared pointers to Entity objects.
     *            The vector is modified directly; inactive entities are removed.
     */
    inline void removeDeadEntities(EntityVec& vec) noexcept
    {
        // Erase elements from the vector where the lambda predicate returns true.
        // Lambda returns true if the entity is inactive, causing its removal.
        std::erase_if(vec,
            [](const std::shared_ptr<Entity>& entity) {
                return !entity->isActive();
            });
    }

public:
    // Require dependencies in the constructor
    EntityManager(SystemManager* sm, ComponentManager* cm)
        : m_systemManager(sm), m_componentManager(cm) {
    }

	EntityManager() = default;

    void setSystemManager(SystemManager* sm) { m_systemManager = sm; }     // Give the EntityManager a reference to the SystemManager so it can notify about changes.
	void setComponentManager(ComponentManager* cm) { m_componentManager = cm; } // Give the EntityManager a reference to the ComponentManager so it can manage components.

    // Find entity by numeric id. Linear search � fine for small projects / prototypes.
    std::shared_ptr<Entity> getEntityById(size_t id) {
        for (auto& e : m_entities) {
            if (e && e->id() == id) return e;
        }
        for (auto& e : m_entitiesToAdd) {
            if (e && e->id() == id) return e;
        }
        return nullptr;
    }

    // Add a component via the manager so systems are notified automatically.
    template<typename T, typename... Args>
    T& addComponent(const std::shared_ptr<Entity>& entity, Args&&... args) {
        auto & comp = m_componentManager->add<T>(entity->id(), std::forward<Args>(args)...);
		entity->signature().set(ComponentType<T>::index, true); // mark component as present in signature (set bit ON)
		if (m_systemManager) m_systemManager->EntitySignatureChanged(entity->id(), entity->signature()); // notify systemManager of signature change
        return comp;
    }

    template<typename T>
    void removeComponent(const std::shared_ptr<Entity>& entity) {
        m_componentManager->remove<T>(entity->id());
		entity->signature().set(ComponentType<T>::index, false); // mark component as absent in signature (set bit OFF)
        if (m_systemManager) m_systemManager->EntitySignatureChanged(entity->id(), entity->signature());
    }

    // Destroy an entity and clean up all ECS state associated with it.
    void destroyEntity(const std::shared_ptr<Entity>& entity) {
        if (!entity) return;

        const auto id = entity->id();

        entity->destroy();

        if (m_componentManager) m_componentManager->entityDestroyed(id);

        entity->signature().reset(); // clear the signature to indicate no components are present

        if (m_systemManager) m_systemManager->EntityDestroyed(entity->id());
    }

    template<typename T>
    bool hasComponent(const std::shared_ptr<Entity>& entity) const {
        return m_componentManager->has<T>(entity->id());
    }

    template<typename T>
    T& getComponent(const std::shared_ptr<Entity>& entity) {
        return m_componentManager->get<T>(entity->id());
    }

    template<typename T>
    const T& getComponent(const std::shared_ptr<Entity>& entity) const {
        return m_componentManager->get<T>(entity->id());
    }

    void update() {
        // Add entities from m_entitiesToAdd to:
        // 1. The main entity vector (m_entities)
        // 2. The tag-indexed map (m_entityMap)
        for (const auto& entity : m_entitiesToAdd) {
            m_entities.push_back(entity);
            m_entityMap[entity->tag()].push_back(entity);

                        // notify systems about this entity's current signature
                if (m_systemManager) {
                    m_systemManager->EntitySignatureChanged(entity->id(), entity->signature());
            }
        }
        m_entitiesToAdd.clear();

        // remove dead entities from the vector of all entities
        removeDeadEntities(m_entities);

        // remove dead entities from each vector in the entity map
        for (auto& [tag, entityVec] : m_entityMap) {
            removeDeadEntities(entityVec);
        }
    }

    std::shared_ptr<Entity> addEntity(const std::string& tag) {
        // Must use 'new' here instead of std::make_shared because Entity's constructor is private.
        // std::make_shared constructs outside EntityManager, so it cannot access private constructors despite friendship.
        std::shared_ptr<Entity> entity(new Entity(m_totalEntities++, tag));
        m_entitiesToAdd.push_back(entity);
        return entity;
    }

    EntityVec& getEntities() {
		return m_entities;
    }

	EntityVec& getEntities(const std::string& tag) {
		return m_entityMap[tag];
	}

	const EntityMap& getEntityMap() const noexcept
    {
		return m_entityMap;
    }

    void clearAll() noexcept {
        // 1) Mark all entities destroyed and notify systems to remove them
        for (const auto& entity : m_entities) {
            if (!entity) continue;
            if (entity->isActive()) entity->destroy();
            if (m_systemManager) m_systemManager->EntityDestroyed(entity->id());
        }

        for (const auto& entity : m_entitiesToAdd) {
            if (!entity) continue;
            if (entity->isActive()) entity->destroy();
            if (m_systemManager) m_systemManager->EntityDestroyed(entity->id());
        }

        // 2) Clear all component storage to avoid stale components
        if (m_componentManager) {
            m_componentManager->clear();
        }

        // 3) Remove all entities from our containers
        m_entities.clear();
        m_entitiesToAdd.clear();
        m_entityMap.clear();

        // 4) Reset entity counter and any ID-recycling structures (if present)
        m_totalEntities = 0;
        // If you later add a free-id vector or id-indexed storage, clear it here too:
        // m_freeIds.clear();
        // m_entitiesById.clear();
    }


};