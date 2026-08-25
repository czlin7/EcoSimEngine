cmake_minimum_required(VERSION 3.28)

project(EcoSimEngine VERSION 0.1.0 LANGUAGES CXX)

option(ECOSIM_BUILD_APP "Build the EcoSimEngine desktop application" ON)

function(ecosim_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

if(ECOSIM_BUILD_APP)
    include(FetchContent)

    find_package(SFML 3.1 QUIET COMPONENTS Graphics Window System Audio)
    if(NOT SFML_FOUND)
        message(STATUS "SFML 3.1 not found locally; fetching SFML 3.1.0")
        set(SFML_BUILD_NETWORK OFF CACHE BOOL "Do not build unused SFML Network module" FORCE)
        set(SFML_USE_SYSTEM_DEPS OFF CACHE BOOL "Use SFML bundled fallback dependencies" FORCE)
        FetchContent_Declare(
            SFML
            GIT_REPOSITORY https://github.com/SFML/SFML.git
            GIT_TAG 3.1.0
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(SFML)
    endif()

    find_package(OpenGL REQUIRED)

    set(ECOSIM_SOURCES
        src/main.cpp
        src/SimulationEngine.cpp
        src/gui/GUIManager.cpp
        src/ecs/Assets.cpp
        src/scene/Scene.cpp
        src/scene/Scene_Menu.cpp
        src/scene/Scene_Simulation.cpp
    )

    set(IMGUI_SOURCES
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui-sfml/imgui-SFML.cpp
    )

    add_executable(EcoSimEngine ${ECOSIM_SOURCES} ${IMGUI_SOURCES})
    target_compile_features(EcoSimEngine PRIVATE cxx_std_20)
    target_include_directories(
        EcoSimEngine
        PRIVATE
            include
            external
            external/imgui
            external/imgui-sfml
            external/nlohmann
    )
    target_link_libraries(
        EcoSimEngine
        PRIVATE
            SFML::Graphics
            SFML::Window
            SFML::System
            SFML::Audio
            OpenGL::GL
    )
    target_compile_definitions(
        EcoSimEngine
        PRIVATE
            $<$<CONFIG:Debug>:DEBUG_BUILD>
    )
    ecosim_enable_warnings(EcoSimEngine)

    add_custom_command(
        TARGET EcoSimEngine
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/config"
                "$<TARGET_FILE_DIR:EcoSimEngine>/config"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/resources"
                "$<TARGET_FILE_DIR:EcoSimEngine>/resources"
        COMMENT "Copying runtime configuration and resources"
    )
endif()

include(CTest)

if(BUILD_TESTING)
    add_executable(
        EcoSimEngineFoundationTests
        tests/foundation_tests.cpp
    )
    target_compile_features(EcoSimEngineFoundationTests PRIVATE cxx_std_20)
    target_include_directories(EcoSimEngineFoundationTests PRIVATE include)
    ecosim_enable_warnings(EcoSimEngineFoundationTests)

    add_test(
        NAME foundation
        COMMAND EcoSimEngineFoundationTests
    )
endif()
