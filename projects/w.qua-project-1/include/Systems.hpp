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
class FPSCameraSystem;
class CollisionSystem;
class RenderSystem;
class InputSystem;
class ObjectManipulationSystem;

namespace Systems
{
    extern std::unique_ptr<InputSystem> g_InputSystem;
    extern std::unique_ptr<FPSCameraSystem> g_CameraSystem;
    extern std::unique_ptr<CollisionSystem> g_CollisionSystem;
    extern std::unique_ptr<RenderSystem> g_RenderSystem;
    extern std::unique_ptr<ObjectManipulationSystem> g_ObjectManipulationSystem;
    extern DemoSceneType g_CurrentDemoScene;

    void InitializeSystems(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader);
    void UpdateSystems(Registry& registry, Window& window, float deltaTime);
    void RenderSystems(Registry& registry, Window& window);
    void ShutdownSystems(Registry& registry);
}