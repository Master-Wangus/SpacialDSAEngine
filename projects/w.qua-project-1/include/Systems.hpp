#pragma once

#include <entt/entt.hpp>
#include "Camera.hpp"
#include "Components.hpp"
#include <memory>

// Forward declarations
class Shader;
class Window;
class Registry;

namespace Systems 
{
    // Scene management functions
    void InitializeSystems(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader);
    void SetupScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader);
    void UpdateSystems(Registry& registry, Window& window, float deltaTime);
    void RenderSystems(Registry& registry, Window& window);
    void ShutdownSystems(Registry& registry);

    // Scene setup helpers
    void SetupCamera(Registry& registry, Window& window);
    void SetupLighting(Registry& registry);
    void CreateCubes(Registry& registry, const std::shared_ptr<Shader>& shader);
    void UpdateTransforms(Registry& registry);
    void UpdateColliders(Registry& registry);
    void DetectCollisions(Registry& registry);
}