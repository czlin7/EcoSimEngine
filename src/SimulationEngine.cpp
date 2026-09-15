#include <iostream>
#include <string>
#include <fstream>

#include <imgui/imgui.h>
#include <imgui-sfml/imgui-SFML.h>
#include <nlohmann/json.hpp>

#include "SFML/Window/Event.hpp"

#include "EcoSimEngine/ecs/Assets.hpp"
#include "EcoSimEngine/SimulationEngine.hpp"
#include "EcoSimEngine/scene/Scene_Menu.hpp"

SimulationEngine::SimulationEngine(const std::string& path)
{
    // Subscribe to GUICommand events early
    m_eventBus.subscribe<Event::GUICommand>([this](const Event::GUICommand& cmd) {
        switch (cmd) {
        case Event::GUICommand::App_Quit: quit(); break;
        case Event::GUICommand::Sim_TogglePause: /* toggle pause */ break;
        case Event::GUICommand::File_NewSimulation: /* start new sim */ break;
        case Event::GUICommand::File_LoadSimulation: /* load sim */ break;
        case Event::GUICommand::File_SaveSimulation: /* save sim */ break;
        default: break;
        }
        });

    init(path);
}

void SimulationEngine::init(const std::string& path) {
    // --- Load JSON config ---
    std::ifstream ifs(path);
    if (!ifs) {
        throw std::runtime_error("Could not open config file: " + path + " in SimulationEngine");
    }

    nlohmann::json json;
    try {
        ifs >> json;
    }
    catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error(std::string("JSON parse error: ") + e.what());
    }

    // --- Parse window config ---
    auto w = json["window"];
    m_windowConfig.width = w["width"].get<unsigned>();
	m_windowConfig.height = w["height"].get<unsigned>();
    m_windowConfig.frameRate = w["frameRate"].get<unsigned>();
	m_windowConfig.vsync = w["vsync"].get<bool>();

    // --- Create window ---
    m_window.create(sf::VideoMode({ m_windowConfig.width, m_windowConfig.height }), "Ecosystem Simulation");
	m_window.setFramerateLimit(m_windowConfig.frameRate);
	m_window.setVerticalSyncEnabled(m_windowConfig.vsync);

    // --- Load assets ---
    try {
    std::string assetsPath = json["paths"]["assets"].get<std::string>();
    m_assets.loadFromFile(assetsPath);
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Asset loading error: ") + e.what());
    }

    // imGui
    m_guiManager = std::make_unique<GUIManager>(this);
    m_guiManager->init(m_window);

    // --- Set initial scene ---
    m_sceneManager.changeScene(SceneID::Menu, std::make_shared<Scene_Menu>(this));
}

std::shared_ptr<Scene> SimulationEngine::currentScene() {
    return m_sceneManager.getCurrentScene();
}

bool SimulationEngine::isRunning() const{
    return m_running && m_window.isOpen();
}

sf::RenderWindow& SimulationEngine::window() {
    return m_window;
}

void SimulationEngine::run() {
    while (isRunning()) {
        sUserInput();

        sf::Time dt = m_deltaClock.restart();
        ImGui::SFML::Update(m_window, dt);

        update();

        if (auto scene = currentScene()) {
            scene->sRender(); // Scene draws SFML content
        }

        // GUI update & build (before ImGui::SFML::Render)
        if (m_guiManager) {
            m_guiManager->update(m_window, dt);
            m_guiManager->render();
        }
        // Scene-specific UI (overlay / inspector)
        if (auto scene = currentScene()) {
            scene->onGui();
        }


        ImGui::SFML::Render(m_window);
        m_window.display();
    }

    if (m_guiManager) {
        m_guiManager->shutdown();
    }
}

void SimulationEngine::sUserInput() {
    while (auto eventOpt = m_window.pollEvent()) {
		auto& event = *eventOpt;
        ImGui::SFML::ProcessEvent(m_window, event); // TO REFACTOR ?

		// Handle window closed
        if (event.is<sf::Event::Closed>()) {
            quit();
            continue;
        }

        /* TODO
        // Screenshot on 'X' key press
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::X) {
                std::cout << "Save screenshot to " << "test.png" << std::endl;
                sf::Texture texture;
                texture.create(m_window.getSize().x, m_window.getSize().y);
                texture.update(m_window);
                if (texture.copyToImage().saveToFile("test.png")) {
                    std::cout << "Screenshot saved to " << "test.png" << std::endl;
                }
            }
        }
        */


        if (auto keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            auto it = currentScene()->getActionMap().find(keyEvent->code);
            if (it != currentScene()->getActionMap().end()) {
                currentScene()->doAction(Action(it->second, ActionType::START));
            }
        }
        else if (auto keyEvent = event.getIf<sf::Event::KeyReleased>()) {
            auto it = currentScene()->getActionMap().find(keyEvent->code);
            if (it != currentScene()->getActionMap().end()) {
                currentScene()->doAction(Action(it->second, ActionType::END));
            }
        }


        // this line ignores mouse events if ImGui is the thing clicked
        //   if (ImGui::GetIO().WantCaptureMouse) { continue; }


        // --- Mouse buttons ---
        auto mousePos = sf::Mouse::getPosition(m_window);
        Vec2f pos = Vec2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        if (auto mb = event.getIf<sf::Event::MouseButtonPressed>()) {
            switch (mb->button) {
            case sf::Mouse::Button::Left:
                currentScene()->doAction(Action(ActionName::LEFT_CLICK, ActionType::START, pos));
                break;
            case sf::Mouse::Button::Middle:
                currentScene()->doAction(Action(ActionName::MIDDLE_CLICK, ActionType::START, pos));
                break;
            case sf::Mouse::Button::Right:
                currentScene()->doAction(Action(ActionName::RIGHT_CLICK, ActionType::START, pos));
                break;
            default:
                break;
            }
        }

        if (auto mb = event.getIf<sf::Event::MouseButtonReleased>()) {
            switch (mb->button) {
            case sf::Mouse::Button::Left:
                currentScene()->doAction(Action(ActionName::LEFT_CLICK, ActionType::END, pos));
                break;
            case sf::Mouse::Button::Middle:
                currentScene()->doAction(Action(ActionName::MIDDLE_CLICK, ActionType::END, pos));
                break;
            case sf::Mouse::Button::Right:
                currentScene()->doAction(Action(ActionName::RIGHT_CLICK, ActionType::END, pos));
                break;
            default:
                break;
            }
        }

        // --- Mouse Move ---
        if (auto mm = event.getIf<sf::Event::MouseMoved>()) {
            currentScene()->doAction(Action(ActionName::MOUSE_MOVE, Vec2f(static_cast<float>(mm->position.x), static_cast<float>(mm->position.y))));
        }
    }
}

void SimulationEngine::quit() {
    m_running = false;
    m_window.close();
}

void SimulationEngine::update() {
    currentScene()->update();
}

Assets& SimulationEngine::assets() {
    return m_assets;
}
