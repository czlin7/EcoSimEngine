#include <iostream>
#include <SFML/Graphics.hpp>
#include "EcoSimEngine/SimulationEngine.hpp"
#include "EcoSimEngine/math/Vec2.hpp"

int main() {
    try {
        std::cout << "[DEBUG] Starting SimulationEngine..." << std::endl;

        SimulationEngine sim("config/config.json");
        std::cout << "[DEBUG] SimulationEngine created successfully." << std::endl;

        sim.run();
        std::cout << "[DEBUG] Simulation run completed successfully." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception caught: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "[ERROR] Unknown exception caught!" << std::endl;
        return -1;
    }

    return 0;
}
