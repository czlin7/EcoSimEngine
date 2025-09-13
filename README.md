# EcoSimEngine
![License](https://img.shields.io/github/license/czlin7/EcoSimEngine)

**EcoSimEngine** is a modular ecosystem simulation engine developed in C++20. It leverages SFML 3.0.0 for graphics rendering and nlohmann::json 3.12.0 for configuration management. The engine employs an Entity-Component-System (ECS) architecture to simulate complex interactions within a 2D environment.

---

### 📦 Features
- ECS Architecture: Entities are defined by components like `CHealth`, `CBehavior`, and `CTransform`.
- Modular Design: Easily extendable with additional components and systems.
- 2D Rendering: Powered by SFML 3.0.0 for graphics, window, and audio handling.
- JSON Configuration: Structured configuration files for environments, species, and simulation settings.
- Cross-platform: Buildable on Windows, Linux, and macOS via CMake.

---

### 🔧 Dependencies
- C++20 compiler (e.g. GCC 11+, Clang 12+, MSVC 19.3+)
- [CMake 3.28+](https://cmake.org/download/)
- [SFML 3.0.0](https://www.sfml-dev.org/download/sfml/3.0.0/) (graphics, window, audio)
  
> ℹ️ The following dependencies are **already included in the project** under `external/`, so you do not need to download them separately:  
> - ImGui  
> - ImGui-SFML  
> - nlohmann/json.hpp  

---

### 🗂 Project Structure
```text
EcoSimEngine/
├── cmake/            # CMake package config (EcoSimEngineConfig.cmake.in, etc.)
├── config/           # JSON configuration files
├── external/         # Third-party libraries (imgui, imgui-sfml, nlohmann)
├── include/          # Public headers (EcoSimEngine API)
│   └── EcoSimEngine/
│       ├── component/
│       ├── ecs/
│       ├── event/
│       ├── gui/
│       ├── math/
│       ├── scene/
│       ├── system/
│       └── utils/
├── resources/        # Assets (fonts, sounds, textures, defaults, definitions)
├── src/              # Engine implementation
├── tests/            # Unit / integration tests (optional)
├── examples/         # Example usage apps (optional)
├── CMakeLists.txt    # Root CMake build script
├── LICENSE           # License file
└── README.md         # Project documentation
```

---

### 🛠 Build & Run

1. Clone the Repository:

   ```
   git clone -b prototype https://github.com/czlin7/EcoSimEngine.git
   cd EcoSimEngine
   ```

2. Configure & build with CMake

   ```
   cmake -B build
   cmake --build build --config Release
   ```

   - CMake will first try to find SFML installed on your system.  
   - **If SFML is not found automatically** (common on Windows), open `CMakeLists.txt` and update the line:
      ```
      set(SFML_DIR "D:/SFML-3.0.0/lib/cmake/SFML") # update to your SFML installation path
      ```

3. Run
   - Executables will be placed in: `build/bin/Debug/` or `build/bin/Release`, depending on the build configuration.

---

### 🤝 Contributing
We welcome contributions to EcoSimEngine! To contribute:

1. **Fork the repository** to your own GitHub account.
2. **Create a new branch** for your feature or bug fix:
```
git checkout -b feature/YourFeatureName
```
3. **Make your changes** in the branch. Follow existing ECS structure and coding style.
4. **Commit your changes** with clear, descriptive messages. Use prefixes like `[feat] ` or `[fix] `.
```
git commit -m "[feat]: Add CBehavior component"
```
5. **Push your branch** to your fork:
```
git push origin feature/YourFeatureName
```
6. **Open a Pull Request** against the `prototype` branch of the main repository.
    - **Guidelines:**
      - Ensure your code builds and passes any existing tests.
      - Keep commits small and focused.
      - Include comments and documentation for new functionality.


### 📄 License
This project is licensed under the [GPL-3.0 License](LICENSE).