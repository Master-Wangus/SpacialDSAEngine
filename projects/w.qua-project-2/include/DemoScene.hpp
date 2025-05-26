/**
 * @class DemoScene
 * @brief Demonstration scene with multiple 3D objects and interaction.
 *
 * This class sets up a 3D scene with various primitive objects, lighting, and
 * interactive components to showcase the engine's capabilities.
 */

#pragma once

#include "pch.h"

// Forward declarations
class Shader;
class Window;
class Registry;

// Define demo scene types
enum class DemoSceneType
{
    Default,
    SphereVsSphere,
    SphereVsAABB,  
    AABBVsAABB,
    PointVsSphere,
    PointVsAABB,
    PointVsTriangle,
    PointVsPlane,
    RayVsPlane,
    RayVsAABB,
    RayVsSphere,
    RayVsTriangle,
    PlaneVsAABB,
    PlaneVsSphere,
    MeshResource  // New demo type for mesh resources
};

namespace DemoScene
{
    // Scene management functions
    void SetupScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType);
    void ClearScene(Registry& registry);
    void SwitchScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType);
    
    // Scene setup helpers for collision demos
    void SetupSphereVsSphereDemo(Registry& registry, const std::shared_ptr<Shader>& shader);
    void SetupAABBVsSphereDemo(Registry& registry, const std::shared_ptr<Shader>& shader);
    void SetupAABBVsAABBDemo(Registry& registry, const std::shared_ptr<Shader>& shader);
    void SetupPointBasedDemos(Registry& registry, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType);
    void SetupRayBasedDemos(Registry& registry, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType);
    void SetupPlaneBasedDemos(Registry& registry, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType);
    void SetupMeshResourceDemo(Registry& registry, const std::shared_ptr<Shader>& shader); // New function
    
    void UpdateTransforms(Registry& registry);
} 