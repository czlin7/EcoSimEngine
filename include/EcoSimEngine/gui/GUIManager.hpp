#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

#include "EcoSimEngine/event/Events.hpp"

#include <memory>

class SimulationEngine;

class GUIManager {
	SimulationEngine* m_engine;
	bool m_initialized{ false };
	bool m_persistentMenuVisible{ true }; // can be toggled by user or scene (e.g. hide during simulation)

	void buildMenuBar();
	void buildOverlays();

	// helpers
	void loaddFonts();

	// syntactic sugar
	void emitEvent(Event::GUICommand cmd);

public:
	explicit GUIManager(SimulationEngine* engine);
	~GUIManager();

	void init(sf::RenderWindow& window);
	void shutdown();

	// called AFTER Imgui::SFML::Update(...)
	void update(const sf::RenderWindow& window, sf::Time dt);

	// Builds all ImGui widgets (main menu, overlays, etc.). Call BEFORE ImGui::SFML::Render
	void render();

	// toggle persistent menu bar (by user or scene)
	void setPersistentMenuVisible(bool v) noexcept { m_persistentMenuVisible = v; }
	bool isPersistentMenuVisible() const noexcept { return m_persistentMenuVisible; }
};
