#include "EcoSimEngine/component/Components.hpp"
#include "EcoSimEngine/component/ComponentManager.hpp"
#include "EcoSimEngine/component/ComponentIndices.hpp"
#include "EcoSimEngine/event/EventBus.hpp"
#include "EcoSimEngine/system/System.hpp"
#include "EcoSimEngine/system/SystemManager.hpp"
#include "EcoSimEngine/ecs/EntityManager.hpp"
#include "EcoSimEngine/simulation/SimulationWorld.hpp"
#include "EcoSimEngine/simulation/SimulationClock.hpp"

#include <iostream>
#include <stdexcept>

namespace
{

    struct Health
    {
        int value;
        explicit Health(int initialValue) : value(initialValue) {}
    };

    struct SimulationSnapshot
    {
        float positionX, positionY;
        float velocityX, velocityY;

        BehaviorState behaviorState;
        float stateTimer;
    };

    class TransformSystem final : public System
    {
    };
    class UnregisteredSystem final : public System
    {
    };

    void require(bool condition, const char *message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    // helper
    SimulationSnapshot runDeterministicSimulation(
    std::uint32_t seed)
{
    SimulationWorld world;

    world.reseed(seed);

    auto& entities =
        world.entityManager();

    auto& components =
        world.componentManager();

    auto entity =
        entities.addEntity("test");

    entities.addComponent<CTransform>(
        entity,
        Vec2f{0.0f, 0.0f});

    entities.addComponent<CBehavior>(
        entity);

    entities.update();

    constexpr float fixedStep =
        1.0f / 60.0f;

    constexpr int ticks = 300;

    for (int i = 0; i < ticks; ++i)
    {
        world.update(fixedStep);
    }

    const auto id =
        entity->id();

    const auto& transform =
        components.get<CTransform>(id);

    const auto& behavior =
        components.get<CBehavior>(id);

    return SimulationSnapshot{
        transform.pos.x,
        transform.pos.y,
        transform.velocity.x,
        transform.velocity.y,
        behavior.current,
        behavior.stateTimer};
}

    void testComponentManager()
    {
        ComponentManager components;

        require(!components.has<Health>(7), "new manager must not contain components");

        auto &health = components.add<Health>(7, 125);
        require(health.value == 125, "added component must keep constructor state");
        require(components.has<Health>(7), "added component must be discoverable");
        require(&components.get<Health>(7) == &health, "get must return stored component");

        bool duplicateRejected = false;
        try
        {
            static_cast<void>(components.add<Health>(7, 90));
        }
        catch (const std::logic_error &)
        {
            duplicateRejected = true;
        }
        require(duplicateRejected, "duplicate component insertion must be rejected");

        components.remove<Health>(7);
        require(!components.has<Health>(7), "removed component must no longer exist");

        bool missingGetRejected = false;
        try
        {
            static_cast<void>(components.get<Health>(7));
        }
        catch (const std::out_of_range &)
        {
            missingGetRejected = true;
        }
        require(missingGetRejected, "getting a missing component must fail explicitly");
    }

    void testSystemManager()
    {
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
        try
        {
            static_cast<void>(systems.RegisterSystem<TransformSystem>());
        }
        catch (const std::logic_error &)
        {
            duplicateRejected = true;
        }
        require(duplicateRejected, "duplicate system registration must fail in release builds too");

        bool unregisteredSignatureRejected = false;
        try
        {
            systems.SetSignature<UnregisteredSystem>(required);
        }
        catch (const std::logic_error &)
        {
            unregisteredSignatureRejected = true;
        }
        require(unregisteredSignatureRejected,
                "setting a signature for an unregistered system must fail explicitly");
    }

    void testEntityDestruction()
    {
        SystemManager systems;
        ComponentManager components;
        EntityManager entities(systems, components);

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

    void testEventBus()
    {
        EventBus bus;
        int total = 0;

        const auto first = bus.subscribe<int>([&](const int &value)
                                              { total += value; });
        bus.subscribe<int>([&](const int &value)
                           { total += value * 10; });

        bus.publish(2);
        require(total == 22, "publish must notify all subscribers");

        bus.unsubscribe(first);
        bus.publish(1);
        require(total == 32, "unsubscribe must remove only the selected subscription");
    }

    void testSimulationClock()
    {
        // A. Not enough elapsed time for one fixed step.
        SimulationClock clock(0.1, 10);

        clock.addElapsed(0.05);

        require(
            !clock.canStep(),
            "clock must not step before enough time has accumulated");

        require(
            clock.tick() == 0,
            "tick must remain zero before a step is consumed");

        // B. Accumulated frame time should eventually produce one step.
        clock.addElapsed(0.05);

        require(
            clock.canStep(),
            "accumulated elapsed time must make one step available");

        clock.consumeStep();

        require(
            clock.tick() == 1,
            "consuming one step must increment the simulation tick");

        require(
            !clock.canStep(),
            "consuming the available step must remove it from the accumulator");

        // C. One frame may produce multiple fixed simulation steps.
        clock.addElapsed(0.25);

        int stepsConsumed = 0;

        while (clock.canStep())
        {
            clock.consumeStep();
            ++stepsConsumed;
        }

        require(
            stepsConsumed == 2,
            "0.25 seconds with a 0.1 second fixed step must produce two complete steps");

        require(
            clock.tick() == 3,
            "simulation tick must count every consumed fixed step");

        // D. Speed changes accumulation rate, not fixed-step size.
        SimulationClock fastClock(0.1, 10);

        fastClock.setSpeed(2.0);
        fastClock.addElapsed(0.05);

        require(
            fastClock.canStep(),
            "2x speed must turn 0.05 real seconds into 0.1 simulation seconds");

        require(
            fastClock.fixedStep() == 0.1,
            "changing simulation speed must not change the fixed timestep");

        fastClock.consumeStep();

        require(
            fastClock.tick() == 1,
            "2x speed clock must consume the resulting simulation step normally");

        // E. maxTicks is a hard upper bound.
        SimulationClock limitedClock(0.1, 3);

        limitedClock.addElapsed(1.0);

        int limitedStepsConsumed = 0;

        while (limitedClock.canStep())
        {
            limitedClock.consumeStep();
            ++limitedStepsConsumed;
        }

        require(
            limitedStepsConsumed == 3,
            "clock must stop after maxTicks even when more accumulated time remains");

        require(
            limitedClock.tick() == 3,
            "tick must stop exactly at maxTicks");

        require(
            limitedClock.finished(),
            "clock must report finished after reaching maxTicks");

        require(
            !limitedClock.canStep(),
            "finished clock must not expose another simulation step");
    }

    void testDeterministicSimulation()
{
    constexpr std::uint32_t seed = 48596;

    const SimulationSnapshot first =
        runDeterministicSimulation(seed);

    const SimulationSnapshot second =
        runDeterministicSimulation(seed);

    require(
        first.positionX == second.positionX &&
        first.positionY == second.positionY,
        "same seed and tick sequence must reproduce entity position");

    require(
        first.velocityX == second.velocityX &&
        first.velocityY == second.velocityY,
        "same seed and tick sequence must reproduce entity velocity");

    require(
        first.behaviorState == second.behaviorState,
        "same seed and tick sequence must reproduce behavior state");

    require(
        first.stateTimer == second.stateTimer,
        "same seed and tick sequence must reproduce behavior timing");
}

} // namespace

int main()
{
    try
    {
        testComponentManager();
        testSystemManager();
        testEntityDestruction();
        testEventBus();
        testSimulationClock();
        testDeterministicSimulation();
    }
    catch (const std::exception &error)
    {
        std::cerr << "foundation test failure: " << error.what() << '\n';
        return 1;
    }

    std::cout << "foundation tests passed\n";
    return 0;
}
