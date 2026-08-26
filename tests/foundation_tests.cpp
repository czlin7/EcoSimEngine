#include "EcoSimEngine/component/ComponentManager.hpp"
#include "EcoSimEngine/component/ComponentIndices.hpp"
#include "EcoSimEngine/event/EventBus.hpp"
#include "EcoSimEngine/system/System.hpp"
#include "EcoSimEngine/system/SystemManager.hpp"
#include "EcoSimEngine/ecs/EntityManager.hpp"

#include <iostream>
#include <stdexcept>

namespace {

struct Health {
    int value;
    explicit Health(int initialValue) : value(initialValue) {}
};

class TransformSystem final : public System {};
class UnregisteredSystem final : public System {};

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testComponentManager() {
    ComponentManager components;

    require(!components.has<Health>(7), "new manager must not contain components");

    auto& health = components.add<Health>(7, 125);
    require(health.value == 125, "added component must keep constructor state");
    require(components.has<Health>(7), "added component must be discoverable");
    require(&components.get<Health>(7) == &health, "get must return stored component");

    bool duplicateRejected = false;
    try {
        static_cast<void>(components.add<Health>(7, 90));
    } catch (const std::logic_error&) {
        duplicateRejected = true;
    }
    require(duplicateRejected, "duplicate component insertion must be rejected");

    components.remove<Health>(7);
    require(!components.has<Health>(7), "removed component must no longer exist");

    bool missingGetRejected = false;
    try {
        static_cast<void>(components.get<Health>(7));
    } catch (const std::out_of_range&) {
        missingGetRejected = true;
    }
    require(missingGetRejected, "getting a missing component must fail explicitly");
}

void testSystemManager() {
    SystemManager systems;
    auto transformSystem = systems.RegisterSystem<TransformSystem>();

    Signature required;
    required.set(0);
    systems.SetSignature<TransformSystem>(required);

    Signature entitySignature;
    systems.EntitySignatureChanged(11, entitySignature);
    require(!transformSystem->mEntities.contains(11),
            "entity without required components must not join system");

    entitySignature.set(0);
    systems.EntitySignatureChanged(11, entitySignature);
    require(transformSystem->mEntities.contains(11),
            "matching entity must join system");

    entitySignature.reset(0);
    systems.EntitySignatureChanged(11, entitySignature);
    require(!transformSystem->mEntities.contains(11),
            "entity must leave system after signature stops matching");

    bool duplicateRejected = false;
    try {
        static_cast<void>(systems.RegisterSystem<TransformSystem>());
    } catch (const std::logic_error&) {
        duplicateRejected = true;
    }
    require(duplicateRejected, "duplicate system registration must fail in release builds too");

    bool unregisteredSignatureRejected = false;
    try {
        systems.SetSignature<UnregisteredSystem>(required);
    } catch (const std::logic_error&) {
        unregisteredSignatureRejected = true;
    }
    require(unregisteredSignatureRejected,
            "setting a signature for an unregistered system must fail explicitly");
}

void testEntityDestruction() {
    SystemManager systems;
    ComponentManager components;
    EntityManager entities(&systems, &components);

    auto transformSystem = systems.RegisterSystem<TransformSystem>();

    Signature required;
    required.set(COMP_INDEX_CTransform);
    systems.SetSignature<TransformSystem>(required);

    auto destroyedEntity = entities.addEntity("destroyed");
    auto survivingEntity = entities.addEntity("survivor");

    entities.addComponent<CTransform>(destroyedEntity);
    entities.addComponent<CEnergy>(destroyedEntity, 50.0f);

    entities.addComponent<CTransform>(survivingEntity);

    entities.update();

    const auto destroyedId = destroyedEntity->id();
    const auto survivingId = survivingEntity->id();

    require(components.has<CTransform>(destroyedId),
            "entity must have Transform before destruction");

    require(components.has<CEnergy>(destroyedId),
            "entity must have Energy before destruction");

    require(transformSystem->mEntities.contains(destroyedId),
            "entity must belong to matching system before destruction");

    entities.destroyEntity(destroyedEntity);

    require(!destroyedEntity->isActive(),
            "destroyed entity must become inactive");

    require(!components.has<CTransform>(destroyedId),
            "destroying entity must remove Transform");

    require(!components.has<CEnergy>(destroyedId),
            "destroying entity must remove Energy");

    require(destroyedEntity->signature().none(),
            "destroying entity must clear its component signature");

    require(!transformSystem->mEntities.contains(destroyedId),
            "destroying entity must remove it from systems");

    require(components.has<CTransform>(survivingId),
            "destroying one entity must not remove another entity's components");

    require(transformSystem->mEntities.contains(survivingId),
            "destroying one entity must not remove another entity from systems");

    entities.update();

    require(entities.getEntityById(destroyedId) == nullptr,
            "destroyed entity must be removed from EntityManager after update");

    require(entities.getEntityById(survivingId) != nullptr,
            "surviving entity must remain managed");
}

void testEventBus() {
    EventBus bus;
    int total = 0;

    const auto first = bus.subscribe<int>([&](const int& value) {
        total += value;
    });
    bus.subscribe<int>([&](const int& value) {
        total += value * 10;
    });

    bus.publish(2);
    require(total == 22, "publish must notify all subscribers");

    bus.unsubscribe(first);
    bus.publish(1);
    require(total == 32, "unsubscribe must remove only the selected subscription");
}

}  // namespace

int main() {
    try {
        testComponentManager();
        testSystemManager();
        testEntityDestruction();
        testEventBus();
    } catch (const std::exception& error) {
        std::cerr << "foundation test failure: " << error.what() << '\n';
        return 1;
    }

    std::cout << "foundation tests passed\n";
    return 0;
}
