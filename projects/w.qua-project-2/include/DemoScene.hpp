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
class Window;
class Registry;

enum class DemoSceneType
{
    MeshScene  
};

namespace DemoScene
{
    void SetupScene(Registry& registry, Window& window, DemoSceneType sceneType = DemoSceneType::MeshScene);
    void ClearScene(Registry& registry);
    void ResetScene(Registry& registry, Window& window);
    
    void SetupMeshScene(Registry& registry);
    
} 