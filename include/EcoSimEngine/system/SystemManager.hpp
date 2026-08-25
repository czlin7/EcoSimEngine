#pragma once

#include "EcoSimEngine/component/ComponentIndices.hpp"
#include "EcoSimEngine/system/System.hpp"

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>

class SystemManager {
    std::unordered_map<std::type_index, Signature> m_signatures;
    std::unordered_map<std::type_index, std::shared_ptr<System>> m_systems;

public:
    SystemManager() {
        m_systems.reserve(16);
        m_signatures.reserve(16);
    }

    template <typename T, typename... Args>
    std::shared_ptr<T> RegisterSystem(Args&&... args) {
        const std::type_index type(typeid(T));
        if (m_systems.contains(type)) {
            throw std::logic_error("System type is already registered");
        }

        auto system = std::make_shared<T>(std::forward<Args>(args)...);
        m_systems.emplace(type, system);
        return system;
    }

    template <typename T>
    void SetSignature(const Signature& signature) {
        const std::type_index type(typeid(T));
        if (!m_systems.contains(type)) {
            throw std::logic_error("Cannot set a signature for an unregistered system");
        }

        m_signatures[type] = signature;
    }

    void EntityDestroyed(EntityId id) {
        for (auto& [_, system] : m_systems) {
            system->mEntities.erase(id);
        }
    }

    void EntitySignatureChanged(EntityId id, const Signature& entitySignature) {
        for (auto& [type, system] : m_systems) {
            const auto signatureIt = m_signatures.find(type);
            if (signatureIt == m_signatures.end()) {
                continue;
            }

            const Signature& systemSignature = signatureIt->second;
            if ((entitySignature & systemSignature) == systemSignature) {
                system->mEntities.insert(id);
            } else {
                system->mEntities.erase(id);
            }
        }
    }

    template <typename T>
    std::shared_ptr<T> GetSystem() {
        const std::type_index type(typeid(T));
        const auto it = m_systems.find(type);
        if (it == m_systems.end()) {
            return nullptr;
        }

        return std::static_pointer_cast<T>(it->second);
    }
};
