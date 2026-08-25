#include "EcoSimEngine/gui/GUIManager.hpp"
#include "EcoSimEngine/SimulationEngine.hpp"
#include "EcoSimEngine/event/Events.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

#include <iostream>
#include <filesystem>


GUIManager::GUIManager(SimulationEngine* engine)
	: m_engine(engine)
{
}

GUIManager::~GUIManager() {
	// don't call shutdown() here � we will call shutdown explicitly when engine stops.
}

void GUIManager::init(sf::RenderWindow& window) {
	if (m_initialized) return;

	// initialize ImGui-SFML but DO NOT auto-load default font
	if (!ImGui::SFML::Init(window, false)) {
		throw std::runtime_error("Failed ImGui-SFML Init in GUIManager");
	}
	m_initialized = true;

	// Load fonts and ensure we have at least one valid font before first frame
	loaddFonts();
}

void GUIManager::shutdown() {
	if (m_initialized) {
		ImGui::SFML::Shutdown();
		m_initialized = false;
	}
}

void GUIManager::update(const sf::RenderWindow& /*window*/, sf::Time /*dt*/) {
	// update persistent states, animations, timers, etc.
}

void GUIManager::render() {
	if (m_persistentMenuVisible) buildMenuBar();
	buildOverlays();
}

void GUIManager::buildMenuBar() {
	if (!ImGui::BeginMainMenuBar()) return;

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("New Simulation")) emitEvent(Event::GUICommand::File_NewSimulation);
		if (ImGui::MenuItem("Load Simulation")) emitEvent(Event::GUICommand::File_LoadSimulation);
		if (ImGui::MenuItem("Save Simulation")) emitEvent(Event::GUICommand::File_SaveSimulation);
		ImGui::Separator();
        if (ImGui::MenuItem("Quit")) emitEvent(Event::GUICommand::App_Quit);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("View")) {
		bool v = m_persistentMenuVisible;
		if (ImGui::MenuItem("Show Menu", nullptr, &v)) {
			m_persistentMenuVisible = v;
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Simulation")) {
		if (ImGui::MenuItem("Pause/Resume (Space)")) emitEvent(Event::GUICommand::Sim_TogglePause);
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void GUIManager::buildOverlays() {
	ImGui::Begin("Status", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("FPS:: %.1f", ImGui::GetIO().Framerate);
	// TODO: show entity counts etc ( ask Scene or System via engine)

	ImGui::End();
}


// ------------------------------------------------------------------
// helpers
// ------------------------------------------------------------------

void GUIManager::loaddFonts() {
	ImGuiIO& io = ImGui::GetIO();

	// Clear any preloaded fonts (safety)
	io.Fonts->Clear();

	// Try to add the asset font, fallback to default if anything fails.
	try {
		std::string fontPath = m_engine->assets().getFontPath("Main");
		std::filesystem::path abs = std::filesystem::absolute(fontPath);
		std::string absStr = abs.string();
		std::replace(absStr.begin(), absStr.end(), '\\', '/'); // normalize

		std::cout << "Attempting to load ImGui font from: " << absStr << std::endl;
		if (!std::filesystem::exists(abs)) {
			throw std::runtime_error("Font file does not exist: " + absStr);
		}

		ImFont* font = io.Fonts->AddFontFromFileTTF(absStr.c_str(), 16.0f);
		if (!font) {
			throw std::runtime_error("ImGui failed to create font face from: " + absStr);
		}

		// success
		std::cout << "Loaded font: " << absStr << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "GUIManager::loaddFonts error: " << e.what()
			<< " � falling back to ImGui default font." << std::endl;
		// Guarantee at least one font exists
		io.Fonts->Clear();
		io.Fonts->AddFontDefault();
	}

	// Debug: confirm number of fonts before updating the texture
	std::cout << "ImGui font count before UpdateFontTexture: " << io.Fonts->Fonts.Size << std::endl;
	if (io.Fonts->Fonts.Size == 0) {
		// defensive: ensure there's always at least one font
		io.Fonts->AddFontDefault();
		std::cout << "Added fallback default font to avoid assertion." << std::endl;
	}

	// Rebuild the font atlas used by ImGui-SFML
	ImGui::SFML::UpdateFontTexture();

	// Confirm everything
	std::cout << "ImGui font count after UpdateFontTexture: " << io.Fonts->Fonts.Size << std::endl;
}


// ------------------------------------------------------------------
// Syntactic Sugar
// ------------------------------------------------------------------

void GUIManager::emitEvent(Event::GUICommand cmd) {
	if (cmd != Event::GUICommand::None) {
		m_engine->eventBus().publish(cmd);
	}
}