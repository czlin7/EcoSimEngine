# EcoSimEngine

![License](https://img.shields.io/github/license/czlin7/EcoSimEngine)

EcoSimEngine is a pre-alpha ecosystem simulation project written in C++20. It currently combines a small Entity-Component-System (ECS), scene management, event dispatch, SFML 3 rendering/audio, Dear ImGui via ImGui-SFML, and JSON-driven simulation resources.

## Current foundation

Implemented areas include:

- entity, component, and system managers;
- component-signature based system membership;
- menu and simulation scenes;
- movement and AI systems;
- a header-only event bus for engine/GUI commands;
- SFML 3 rendering and audio;
- ImGui-SFML GUI integration;
- JSON configuration, species definitions, and default simulation data;
- CMake-based builds;
- small headless foundation tests for core managers and the event bus.

The project is still under active development. Save/load, simulation behaviour, GUI workflows, and test coverage are not yet complete.

## Requirements

- CMake 3.28+
- a C++20-capable compiler
- Git when CMake needs to fetch SFML

The repository vendors Dear ImGui, ImGui-SFML, and nlohmann/json. CMake first looks for an installed SFML 3.1 package and otherwise fetches SFML 3.1.0 during configuration. Only the SFML modules used by EcoSimEngine are built.

## Build

Configure from the repository root:

```bash
cmake -S . -B build
cmake --build build --config Release
```

For single-configuration generators such as Unix Makefiles or Ninja, choose the build type during configuration:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

After the application target is built, CMake copies `config/` and `resources/` next to the executable so the current relative runtime paths continue to work.

## Tests

Run the foundation tests after configuring:

```bash
ctest --test-dir build -C Release --output-on-failure
```

The core tests can also be configured without SFML or the desktop application:

```bash
cmake -S . -B build-tests \
  -DECOSIM_BUILD_APP=OFF \
  -DBUILD_TESTING=ON \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

This headless path is used by CI to validate the dependency-free core managers and event bus. CI also builds the complete desktop application on Linux and Windows.

## Project structure

```text
EcoSimEngine/
├── .github/workflows/   # CI
├── config/              # Runtime JSON configuration
├── external/            # Vendored ImGui, ImGui-SFML, nlohmann/json
├── include/EcoSimEngine/
│   ├── gui/
│   ├── component/
│   ├── ecs/
│   ├── event/
│   ├── math/
│   ├── scene/
│   ├── system/
│   └── utils/
├── resources/           # Assets, defaults, and simulation definitions
├── src/                 # Application implementation
├── tests/               # Headless foundation tests
├── CMakeLists.txt
├── LICENSE
└── README.md
```

## Development status

The current goal is to keep the foundation small, understandable, and buildable while extending the simulation incrementally. New work should preserve clear ownership between the engine, ECS managers, scenes, systems, and GUI rather than adding abstraction without a concrete need.

## License

EcoSimEngine is licensed under the GPL-3.0 License. See `LICENSE`.
