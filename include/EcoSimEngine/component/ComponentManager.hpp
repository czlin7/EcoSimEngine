#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>

class ComponentManager {
    struct IStore {
        virtual ~IStore() = default;
    };

    template <typename T>
    struct Store final : IStore {
        std::unordered_map<std::size_t, T> data;
    };

    std::unordered_map<std::type_index, std::unique_ptr<IStore>> m_stores;

    template <typename T>
    Store<T>& ensureStore() {
        const auto type = std::type_index(typeid(T));
        auto it = m_stores.find(type);
        if (it == m_stores.end()) {
            it = m_stores.emplace(type, std::make_unique<Store<T>>()).first;
        }
        return *static_cast<Store<T>*>(it->second.get());
    }

    template <typename T>
    Store<T>* findStore() noexcept {
        const auto it = m_stores.find(std::type_index(typeid(T)));
        if (it == m_stores.end()) {
            return nullptr;
        }
        return static_cast<Store<T>*>(it->second.get());
    }

    template <typename T>
    const Store<T>* findStore() const noexcept {
        const auto it = m_stores.find(std::type_index(typeid(T)));
        if (it == m_stores.end()) {
            return nullptr;
        }
        return static_cast<const Store<T>*>(it->second.get());
    }

public:
    template <typename T, typename... Args>
    T& add(std::size_t id, Args&&... args) {
        auto& store = ensureStore<T>();
        auto [it, inserted] =
            store.data.emplace(id, T(std::forward<Args>(args)...));

        if (!inserted) {
            throw std::logic_error("Entity already has this component type");
        }

        return it->second;
    }

    template <typename T>
    [[nodiscard]] bool has(std::size_t id) const noexcept {
        const auto* store = findStore<T>();
        return store != nullptr && store->data.contains(id);
    }

    template <typename T>
    T& get(std::size_t id) {
        auto* store = findStore<T>();
        if (store == nullptr) {
            throw std::out_of_range("Component type is not registered");
        }
        return store->data.at(id);
    }

    template <typename T>
    const T& get(std::size_t id) const {
        const auto* store = findStore<T>();
        if (store == nullptr) {
            throw std::out_of_range("Component type is not registered");
        }
        return store->data.at(id);
    }

    template <typename T>
    void remove(std::size_t id) noexcept {
        auto* store = findStore<T>();
        if (store != nullptr) {
            store->data.erase(id);
        }
    }

    void clear() noexcept {
        m_stores.clear();
    }
};
