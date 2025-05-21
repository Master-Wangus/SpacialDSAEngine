#pragma once

#include <entt/entt.hpp>
#include "Components.hpp"
#include <memory>

// Forward declarations
class Shader;
class Window;
class Registry;
class FPSCameraSystem;
class CollisionSystem;
class RenderSystem;

namespace Systems
{
    // System instances
    extern std::unique_ptr<FPSCameraSystem> g_CameraSystem;
    extern std::unique_ptr<CollisionSystem> g_CollisionSystem;
    extern std::unique_ptr<RenderSystem> g_RenderSystem;

    // Scene management functions
    void InitializeSystems(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader);
    void SetupScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader);
    void UpdateSystems(Registry& registry, Window& window, float deltaTime);
    void RenderSystems(Registry& registry, Window& window);
    void ShutdownSystems(Registry& registry);

    // Scene setup helpers
    void CreateCubes(Registry& registry, const std::shared_ptr<Shader>& shader);
    void UpdateTransforms(Registry& registry);
}