/**
 * @class DemoScene
 * @brief Demonstration scene for rendering 3D mesh objects.
 *
 * This class sets up a 3D scene with mesh objects and lighting to showcase
 * the engine's mesh rendering capabilities.
 */

#pragma once

#include "pch.h"

// Forward declarations
class Shader;
class Window;
class Registry;

enum class DemoSceneType
{
    MeshScene  
};

namespace DemoScene
{
    // Scene management functions
    void SetupScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType = DemoSceneType::MeshScene);
    void ClearScene(Registry& registry);
    
    // Scene setup for mesh rendering
    void SetupMeshScene(Registry& registry, const std::shared_ptr<Shader>& shader);
    
} 