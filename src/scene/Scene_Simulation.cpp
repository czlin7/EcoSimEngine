#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <random>

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Event.hpp"

#include <imgui/imgui.h>
#include <imgui-sfml/imgui-SFML.h>
#include <nlohmann/json.hpp>

#include "EcoSimEngine/component/Components.hpp"
#include "EcoSimEngine/SimulationEngine.hpp"
#include "EcoSimEngine/scene/Scene_Menu.hpp"
#include "EcoSimEngine/scene/Scene_Simulation.hpp"
#include "EcoSimEngine/math/Vec2.hpp"
#include "EcoSimEngine/utils/Utils.hpp"
#include "EcoSimEngine/utils/SpatialHash.hpp"
#include "EcoSimEngine/system/MovementSystem.hpp"
#include "EcoSimEngine/system/AISystem.hpp"


Scene_Simulation::Scene_Simulation(SimulationEngine* engine, const std::string& simKey)
    : Scene(engine)
    , m_simKey(simKey)
{
    init(simKey);
}

void Scene_Simulation::init(const std::string& simulationKey) {
    // Decide whether to load a named simulation (if provided) or default
    if (!simulationKey.empty()) {
        loadSimulation(simulationKey);
    }
    else {
        loadDefaultSimulation(m_defaultSimulationPath);
    }

    // MAYDELETE
    // m_gridText.setCharacterSize(12);
    // m_gridText.setFont(m_simulation->assets().getFont("Main"));

    registerAction(sf::Keyboard::Key::Escape, ActionName::QUIT_AND_SAVE);
    registerAction(sf::Keyboard::Key::P, ActionName::PAUSE);
    registerAction(sf::Keyboard::Key::Y, ActionName::TOGGLE_FOLLOW);    // toggle follow camera
    registerAction(sf::Keyboard::Key::T, ActionName::TOGGLE_TEXTURE);   // toggle drawing (T)extures
    registerAction(sf::Keyboard::Key::C, ActionName::TOGGLE_COLLISION); // toggle drawing (C)ollision Box
    registerAction(sf::Keyboard::Key::G, ActionName::TOGGLE_GRID);      // toggle drawing (G)rid

    // Register the actions required to simulation control
    registerAction(sf::Keyboard::Key::W, ActionName::UP);
    registerAction(sf::Keyboard::Key::S, ActionName::DOWN);
    registerAction(sf::Keyboard::Key::A, ActionName::LEFT);
    registerAction(sf::Keyboard::Key::D, ActionName::RIGHT);
}

void Scene_Simulation::loadSimulation(const std::string& simulationKey) {
    // TODO: implement full named-save loading; for now attempt to open a file, fallback to default.
    const auto path = buildSavePathFromKey(simulationKey);
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "No save found for key \"" << simulationKey
            << "\" at " << path << " � loading default.\n";
        loadDefaultSimulation(m_defaultSimulationPath);
        return;
    }

    nlohmann::json simJson;
    try {
        ifs >> simJson;
    }
    catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Failed to parse save file " << path << ": " << e.what()
            << " � loading default.\n";
        loadDefaultSimulation(m_defaultSimulationPath);
        return;
    }

    // Clear any existing entities
    m_simulation->entityManager().clearAll();
    spawnFromJson(simJson);
}

void Scene_Simulation::loadDefaultSimulation(const std::string& defaultSimulationPath) {
    std::ifstream ifs(defaultSimulationPath);
    if (!ifs.is_open()) {
        std::cerr << "Error: Could not open default simulation file: " << defaultSimulationPath << std::endl;
        return;
    }

    nlohmann::json simJson;
    try {
        ifs >> simJson;
    }
    catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Failed to parse default simulation file " << defaultSimulationPath << ": " << e.what() << "\n";
        return;
    }

    // Clear any existing entities
    m_simulation->entityManager().clearAll();
    spawnFromJson(simJson);
}

void Scene_Simulation::spawnFromJson(const nlohmann::json& simJson) {
    auto& em = m_simulation->entityManager();
    auto& cm = m_simulation->componentManager();

    // read population counts
    auto populations = simJson["simulation"]["initialPopulation"];

    // --- set up RNG once per load ---
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distX(
        0, simJson["simulation"]["world"]["size"]["width"].get<int>());
    std::uniform_real_distribution<float> distY(
        0, simJson["simulation"]["world"]["size"]["height"].get<int>());

    int speciesCount = static_cast<int>(populations.size());

    // Precompute species colours (single pass)
    int idx = 0;
    for (auto& [speciesName, popData] : populations.items()) {
        float hue = (idx * 360.0f) / std::max(1, speciesCount);  // evenly spaced hues
        m_speciesColors[speciesName] = hslToRgb(hue, 0.7f, 0.5f); // 70% sat, 50% lightness
        ++idx;
    }

    for (auto& [speciesName, popData] : populations.items()) {
        int total = popData["total"];
        int males = popData["male"];
        int females = popData["female"];

        // load species data JSON (per-species data)
        std::string speciesFile = "resources/definitions/species/" + speciesName + ".json";
        std::ifstream sf(speciesFile);
        if (!sf.is_open()) {
            std::cerr << "Could not open species file: " << speciesFile << "\n";
            continue;
        }
        nlohmann::json speciesJson;
        sf >> speciesJson;

        // create entities
        for (int i = 0; i < total; ++i) {
            auto entity = em.addEntity(speciesName);

            // --- add Components ---
            em.addComponent<CSpecies>(entity, speciesName, 0); // default age is 0
            em.addComponent<CHealth>(entity, 100.0f);
            em.addComponent<CEnergy>(entity, 100.0f);

            // proper random position
            float x = distX(gen);
            float y = distY(gen);
            em.addComponent<CTransform>(entity, Vec2f(x, y));

            // reproductive component
            auto& repro = em.addComponent<CReproductive>(entity);
            repro.sex = (i < males ? Sex::Male : Sex::Female);
            repro.canReproduce = true;

            // TODO
            em.addComponent<CBehavior>(entity);
        }
        std::cout << "Loaded " << total << " " << speciesName << " entities.\n";
    }

    // Now flush pending entities -> this also calls EntitySignatureChanged for newly added entities
    em.update();
}

void Scene_Simulation::sDoAction(const Action& action) {
    if (action.type() != ActionType::START) return;

    if (action.name() == ActionName::QUIT_AND_SAVE)
    {
        onEnd();
    }

}

void Scene_Simulation::update() {
    auto& em = m_simulation->entityManager();
    auto& cm = m_simulation->componentManager();

    em.update();

    float dt = m_clock.restart().asSeconds();
    if (dt <= 0.0f) dt = 1.0f / 60.0f;

    // --- rebuild spatial hash every tick (fast) ---
    m_spatialHash.clear();
    for (const auto& ent : em.getEntities()) {
        if (!ent || !ent->isActive()) continue;
        if (!cm.has<CTransform>(ent->id())) continue;
        const auto& t = cm.get<CTransform>(ent->id());
        m_spatialHash.insert(ent, t.pos.x, t.pos.y);
    }

    // Implement pause functionality
    if (!m_paused) {
        //sDrag(); // MAYDELETE
       // call AI first, then Movement
        if (auto ais = m_simulation->systemManager().GetSystem<AISystem>()) {
            ais->update(em, cm, dt);
        }
        if (auto ms = m_simulation->systemManager().GetSystem<MovementSystem>()) {
            ms->update(em, cm, dt);
        }
        //sStatus();
        //sCollision();
        //sAnimation();
        //sCamera();
        m_currentFrame++;
    }
}

void Scene_Simulation::onEnd() {
    // When the scene ends, change back to the MENU scene

    // Important: pass m_simulation (the SimulationEngine context) to Scene_Menu,
    // not "this". Scenes should always receive the engine that owns them so they
    // can access window, assets, input, etc. Passing "this" (the current scene)
    // would be wrong because Scene_Menu is not constructed from another scene.
    m_simulation->sceneManager()
        .changeScene(SceneID::Menu, std::make_shared<Scene_Menu>(m_simulation));
}


void Scene_Simulation::sRender() {
    auto& em = m_simulation->entityManager();
    auto& cm = m_simulation->componentManager();

    sf::RenderWindow& win = m_simulation->window();
    win.clear(sf::Color(15, 15, 20));

    for (const auto& entity : em.getEntities()) {
        if (!entity->isActive()) continue;
        const auto id = entity->id();
        if (!cm.has<CTransform>(id)) continue;
        const auto& transform = cm.get<CTransform>(id);

        sf::CircleShape circle(10.0f);
        circle.setOrigin({ circle.getRadius(), circle.getRadius() });
        circle.setPosition({ transform.pos.x, transform.pos.y });

        // lookup species color
        const auto& species = cm.get<CSpecies>(id);
        auto it = m_speciesColors.find(species.speciesName);
        circle.setFillColor(it != m_speciesColors.end() ? it->second : sf::Color::White);

        win.draw(circle);
    }
}

void Scene_Simulation::onGui() {
    // Example overlay window showing simulation info
    ImGui::Begin("Simulation Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Frame/time info
    ImGui::Text("Frame: %zu", m_currentFrame);
    ImGui::Text("Delta time: %.3f s", m_clock.getElapsedTime().asSeconds());

    // Mouse info
    ImGui::Text("Mouse Position: (%.1f, %.1f)", m_mousePos.x, m_mousePos.y);

    // Toggles for debug rendering
    ImGui::Checkbox("Draw Textures", &m_drawTextures);
    ImGui::Checkbox("Draw Collision", &m_drawCollision);
    ImGui::Checkbox("Draw Grid", &m_drawGrid);
    ImGui::Checkbox("Follow Entity", &m_follow);

    // Example of inspecting a selected entity (if you integrate DebugUISystem later)
    // auto selected = m_simulation->debugSystem().selectedEntity();
    // ImGui::Text("Selected entity: %zu", selected);

    ImGui::End();
}


// helpers

// simple mapping from a simulation key/name to a file path.
// Right now we just look in a "saves/" folder; you can change this.
std::string Scene_Simulation::buildSavePathFromKey(const std::string& key) {
    return "saves/" + key + ".json";
}
