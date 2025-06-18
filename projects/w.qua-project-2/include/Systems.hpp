/**
 * @file Systems.hpp
 * @brief System management and coordination for the game engine.
 *
 * This file provides centralized management of all systems in the engine, including
 * initialization, update, rendering, and shutdown of system components.
 */

#pragma once

#include "pch.h"
#include "Components.hpp"
#include "DemoScene.hpp"

// Forward declarations
class Shader;
class Window;
class Registry;
class CameraSystem;
class RenderSystem;
class InputSystem;
class EventSystem;

namespace Systems
{
    extern std::unique_ptr<InputSystem> g_InputSystem;
    extern std::unique_ptr<CameraSystem> g_CameraSystem;
    extern std::unique_ptr<RenderSystem> g_RenderSystem;
    extern DemoSceneType g_CurrentDemoScene;

    /**
     * @brief Initializes all engine systems.
     * @param registry Entity registry for system initialization
     * @param window Window reference for system setup
     * @param shader Shared pointer to the main shader program
     */
    void InitializeSystems(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader);
    
    /**
     * @brief Updates all engine systems each frame.
     * @param registry Entity registry for system updates
     * @param window Window reference for system updates
     * @param deltaTime Time elapsed since last frame
     */
    void UpdateSystems(Registry& registry, Window& window, float deltaTime);
    
    /**
     * @brief Renders all engine systems.
     * @param registry Entity registry for rendering
     * @param window Window reference for rendering context
     */
    void RenderSystems(Registry& registry, Window& window);
    
    /**
     * @brief Shuts down all engine systems and cleans up resources.
     * @param registry Entity registry for system cleanup
     */
    void ShutdownSystems(Registry& registry);
    
    /**
     * @brief Resets the current scene to its initial state.
     * @param registry Entity registry to reset
     * @param window Window reference for scene reset
     */
    void ResetCurrentScene(Registry& registry, Window& window);
}