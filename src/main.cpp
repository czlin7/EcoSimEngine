#include "EcoSimEngine/SimulationEngine.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
    try {
        SimulationEngine simulation("config/config.json");
        simulation.run();
    } catch (const std::exception& error) {
        std::cerr << "EcoSimEngine failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "EcoSimEngine failed with an unknown error\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
