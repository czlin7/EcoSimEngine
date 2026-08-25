#pragma once

#include <string>
#include <unordered_map>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <nlohmann/json.hpp>

#include "EcoSimEngine/component/Components.hpp"
#include "EcoSimEngine/scene/Scene.hpp"
#include "EcoSimEngine/utils/SpatialHash.hpp"

class Scene_Simulation : public Scene {
private:
    std::string m_simKey;
    const std::string m_defaultSimulationPath{ "resources/defaults/default_simulation.json" };

    std::unordered_map<std::string, sf::Color> m_speciesColors;
    SpatialHash m_spatialHash{ 120.0f };
    sf::Clock m_clock;

protected:
    bool m_drawTextures{ true };
    bool m_drawCollision{};
    bool m_drawGrid{};
    bool m_follow{};

    const Vec2f m_gridSize{ 64.0f, 64.0f };
    Vec2f m_mousePos;

    void init(const std::string& simulationKey);
    void loadSimulation(const std::string& simulationKey);
    void loadDefaultSimulation(const std::string& defaultSimulationPath);
    void spawnFromJson(const nlohmann::json& simJson);

    void onEnd() override;
    void sDoAction(const Action& action) override;
    void sRender() override;

public:
    explicit Scene_Simulation(
        SimulationEngine* simulationEngine,
        const std::string& simKey = {});

    void update() override;
    std::string buildSavePathFromKey(const std::string& key);
    void onGui() override;
};
