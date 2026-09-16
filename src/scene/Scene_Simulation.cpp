#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

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

#include "EcoSimEngine/utils/ColorUtils.hpp"
#include "EcoSimEngine/utils/Random.hpp"

Scene_Simulation::Scene_Simulation(SimulationEngine *engine, const std::string &simKey)
    : Scene(engine), m_simKey(simKey)
{
    init(simKey);
}

void Scene_Simulation::init(const std::string &simulationKey)
{
    // Decide whether to load a named simulation (if provided) or default
    if (!simulationKey.empty())
    {
        loadSimulation(simulationKey);
    }
    else
    {
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

void Scene_Simulation::loadSimulation(const std::string &simulationKey)
{
    // TODO: implement full named-save loading; for now attempt to open a file, fallback to default.
    const auto path = buildSavePathFromKey(simulationKey);
    std::ifstream ifs(path);
    if (!ifs.is_open())
    {
        std::cerr << "No save found for key \"" << simulationKey
                  << "\" at " << path << " � loading default.\n";
        loadDefaultSimulation(m_defaultSimulationPath);
        return;
    }

    nlohmann::json simJson;
    try
    {
        ifs >> simJson;
    }
    catch (const nlohmann::json::parse_error &e)
    {
        std::cerr << "Failed to parse save file " << path << ": " << e.what()
                  << " � loading default.\n";
        loadDefaultSimulation(m_defaultSimulationPath);
        return;
    }

    // Clear any existing entities
    m_world.entityManager().clearAll();
    spawnFromJson(simJson);
}

void Scene_Simulation::loadDefaultSimulation(const std::string &defaultSimulationPath)
{
    std::ifstream ifs(defaultSimulationPath);
    if (!ifs.is_open())
    {
        std::cerr << "Error: Could not open default simulation file: " << defaultSimulationPath << std::endl;
        return;
    }

    nlohmann::json simJson;
    try
    {
        ifs >> simJson;
    }
    catch (const nlohmann::json::parse_error &e)
    {
        std::cerr << "Failed to parse default simulation file " << defaultSimulationPath << ": " << e.what() << "\n";
        return;
    }

    // Clear any existing entities
    m_world.entityManager().clearAll();
    spawnFromJson(simJson);
}

void Scene_Simulation::spawnFromJson(const nlohmann::json &simJson)
{
    auto &em = m_world.entityManager();

    const auto &simulationJson = simJson.at("simulation");
    const auto &worldJson = simulationJson.at("world");

    // --- configure deterministic RNG ---
    const std::uint32_t seed =
        simulationJson.at("seed").get<std::uint32_t>();

    m_world.reseed(seed);

    // --- configure simulation clock ---
    const double fixedStep =
        worldJson.at("timeStep").get<double>();

    const std::uint64_t maxTicks =
        worldJson.at("maxTicks").get<std::uint64_t>();

    m_simulationClock.configure(fixedStep, maxTicks);

    // --- load world dimensions ---
    const float worldWidth =
        worldJson.at("size").at("width").get<float>();

    const float worldHeight =
        worldJson.at("size").at("height").get<float>();

    const auto &populations =
        simulationJson.at("initialPopulation");

    const int speciesCount =
        static_cast<int>(populations.size());

    // Precompute species colours (single pass)
    int idx = 0;

    for (const auto &[speciesName, popData] : populations.items())
    {
        const float hue =
            (idx * 360.0f) / std::max(1, speciesCount); // evenly spaced hues

        m_speciesColors[speciesName] =
            hslToRgb(hue, 0.7f, 0.5f); // 70% sat, 50% lightness

        ++idx;
    }

    for (const auto &[speciesName, popData] : populations.items())
    {
        const int total = popData.at("total").get<int>();
        const int males = popData.at("male").get<int>();
        const int females = popData.at("female").get<int>();

        // load species data JSON (per-species data)
        std::string speciesFile =
            "resources/definitions/species/" + speciesName + ".json";

        std::ifstream sf(speciesFile);

        if (!sf.is_open())
        {
            std::cerr << "Could not open species file: "
                      << speciesFile << '\n';

            continue;
        }

        nlohmann::json speciesJson;
        sf >> speciesJson;

        // create entities
        for (int i = 0; i < total; ++i)
        {
            auto entity = em.addEntity(speciesName);

            // --- add Components ---
            em.addComponent<CSpecies>(entity, speciesName, 0); // default age is 0
            em.addComponent<CHealth>(entity, 100.0f);
            em.addComponent<CEnergy>(entity, 100.0f);

            // proper random position
            const float x =
                randomFloat(
                    m_world.rng(),
                    0.0f,
                    worldWidth);

            const float y =
                randomFloat(
                    m_world.rng(),
                    0.0f,
                    worldHeight);

            em.addComponent<CTransform>(entity, Vec2f{x, y});

            // reproductive component
            auto &reproductive =
                em.addComponent<CReproductive>(entity);

            reproductive.sex =
                (i < males ? Sex::Male : Sex::Female);

            reproductive.canReproduce = true;

            em.addComponent<CBehavior>(entity);
        }

        std::cout << "Loaded "
                  << total << " "
                  << speciesName
                  << " entities.\n";
    }

    // Now flush pending entities -> this also calls EntitySignatureChanged for newly added entities
    em.update();
}

void Scene_Simulation::sDoAction(const Action &action)
{
    if (action.type() != ActionType::START)
        return;

    if (action.name() == ActionName::QUIT_AND_SAVE)
    {
        onEnd();
        return;
    }

    if (action.name() == ActionName::PAUSE)
    {
        setPaused(!m_paused);
        return;
    }
}

void Scene_Simulation::update()
{
    const float realDeltaSeconds = m_clock.restart().asSeconds();

    // Always restart the real-time clock, including while paused.
    // Otherwise time spent paused would be accumulated and processed
    // as a large catch-up burst after unpausing.
    if (m_paused)
    {
        return;
    }

    if (m_simulationClock.finished())
    {
        return;
    }

    m_simulationClock.addElapsed(
        static_cast<double>(realDeltaSeconds));

    while (m_simulationClock.canStep())
    {
        m_world.update(
            static_cast<float>(m_simulationClock.fixedStep()));

        m_simulationClock.consumeStep();
    }

    // Keep the legacy Scene frame counter aligned with simulation
    // progression for now. SimulationClock::tick() is the authoritative
    // simulation counter.
    m_currentFrame =
        static_cast<std::size_t>(
            m_simulationClock.tick());
}

void Scene_Simulation::onEnd()
{
    // When the scene ends, change back to the MENU scene

    // Important: pass m_simulation (the SimulationEngine context) to Scene_Menu,
    // not "this". Scenes should always receive the engine that owns them so they
    // can access window, assets, input, etc. Passing "this" (the current scene)
    // would be wrong because Scene_Menu is not constructed from another scene.
    m_simulation->sceneManager()
        .changeScene(SceneID::Menu, std::make_shared<Scene_Menu>(m_simulation));
}

void Scene_Simulation::sRender()
{
    auto &em = m_world.entityManager();
    auto &cm = m_world.componentManager();

    sf::RenderWindow &win = m_simulation->window();
    win.clear(sf::Color(15, 15, 20));

    for (const auto &entity : em.getEntities())
    {
        if (!entity->isActive())
            continue;
        const auto id = entity->id();
        if (!cm.has<CTransform>(id))
            continue;
        const auto &transform = cm.get<CTransform>(id);

        sf::CircleShape circle(10.0f);
        circle.setOrigin({circle.getRadius(), circle.getRadius()});
        circle.setPosition({transform.pos.x, transform.pos.y});

        // lookup species color
        const auto &species = cm.get<CSpecies>(id);
        auto it = m_speciesColors.find(species.speciesName);
        circle.setFillColor(it != m_speciesColors.end() ? it->second : sf::Color::White);

        win.draw(circle);
    }
}

void Scene_Simulation::onGui()
{
    // Example overlay window showing simulation info
    ImGui::Begin(
        "Simulation Info ",
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text(
        "Tick: %llu / %llu",
        static_cast<unsigned long long>(m_simulationClock.tick()),
        static_cast<unsigned long long>(m_simulationClock.maxTicks()));

    ImGui::Text(
        "Fixed timestep: %.6f s",
        m_simulationClock.fixedStep());

    ImGui::Text(
        "Paused: %s",
        m_paused ? "yes" : "no");

    if (m_simulationClock.finished())
    {
        ImGui::Text("Simulation finished.");
    }

    float simulationSped = static_cast<float>(m_simulationClock.speed());

    if (ImGui::SliderFloat(
            "Simulation Speed",
            &simulationSped,
            0.25f,
            4.0f,
            "%.2fx"))
    {
        m_simulationClock.setSpeed(static_cast<double>(simulationSped));
    }

    ImGui::Separator();

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
std::string Scene_Simulation::buildSavePathFromKey(const std::string &key)
{
    return "saves/" + key + ".json";
}
