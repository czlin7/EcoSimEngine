cmake_minimum_required(VERSION 3.28)
project(EcoSimEngine VERSION 0.1.0 LANGUAGES CXX)

# ---------------------
# C++ standard settings
# ---------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Avoid compiler-specific extensions

# ---------------------
# Output folders
# ---------------------
# All executables will be placed in build/bin, with subfolders per configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)

# ---------------------
# Source files
# ---------------------
# Glob all project and ImGui/ImGui-SFML source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS
    external/imgui/*.cpp
    external/imgui/backends/*.cpp
    external/imgui-sfml/*.cpp
)

# Define executable
add_executable(EcoSimEngine ${PROJECT_SOURCES} ${IMGUI_SOURCES})

# ---------------------
# Include directories
# ---------------------
# Use PRIVATE to prevent leaking internal includes to dependents
target_include_directories(EcoSimEngine PRIVATE
    include
    external
    external/imgui
    external/imgui-sfml
    external/nlohmann
)

# ---------------------
# SFML 3 setup
# ---------------------
include(FetchContent)

# Try to find SFML installed on the system first
find_package(SFML 3 COMPONENTS Graphics Window System Audio QUIET)

# On Windows, allow manual override of SFML_DIR
if(NOT SFML_FOUND AND WIN32)
    set(SFML_DIR "D:/SFML-3.0.0/lib/cmake/SFML") # <-- Update path to local SFML if needed
    find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)
endif()

# Fallback: fetch SFML via FetchContent if not found
if(NOT SFML_FOUND)
    message(STATUS "SFML not found, fetching via FetchContent...")
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.0
    )
    FetchContent_MakeAvailable(SFML)
endif()

# Link SFML libraries to the executable
target_link_libraries(EcoSimEngine PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)

# OpenGL needed for ImGui-SFML
if(WIN32)
    target_link_libraries(EcoSimEngine PRIVATE opengl32)
endif()

# ---------------------
# Copy config and resources to output
# ---------------------
# Ensures executable has access to config files and assets for both Debug/Release
foreach(CONFIG_TYPE Debug Release)
    file(COPY ${CMAKE_SOURCE_DIR}/config DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG_TYPE})
    file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG_TYPE})
endforeach()

# ---------------------
# Compiler warning settings
# ---------------------
if(MSVC)
    target_compile_options(EcoSimEngine PRIVATE /W4 /permissive-) # High warning level, disable permissive MSVC
else()
    target_compile_options(EcoSimEngine PRIVATE -Wall -Wextra -pedantic) # Enable common warnings for GCC/Clang
endif()

# ---------------------
# Post-build DLL copy (Windows only)
# ---------------------
# Ensures SFML DLLs are available next to the executable
if(WIN32 AND SFML_FOUND)
    function(copy_sfml_dlls target)
        foreach(config Debug Release)
            if(config STREQUAL "Debug")
                set(dll_suffix "-d")
            else()
                set(dll_suffix "")
            endif()

            foreach(lib graphics window system audio)
                set(dll "${SFML_DIR}/../../../bin/sfml-${lib}-3${dll_suffix}.dll")
                if(EXISTS ${dll})
                    add_custom_command(TARGET ${target} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            ${dll} $<TARGET_FILE_DIR:${target}>
                        COMMENT "Copying SFML ${lib} DLL for ${config}"
                    )
                endif()
            endforeach()
        endforeach()
    endfunction()

    copy_sfml_dlls(EcoSimEngine)
endif()

# ---------------------
# Debug defines
# ---------------------
# Allows code to conditionally compile debug-specific code
if(MSVC)
    target_compile_definitions(EcoSimEngine PRIVATE DEBUG_BUILD)
else()
    target_compile_definitions(EcoSimEngine PRIVATE DEBUG_BUILD)
endif()
# End of CMakeLists.txt