#pragma once

#include "pch.h"
#include "Components.hpp"

// Forward declarations
class Shader;
class Window;
class Registry;
class FPSCameraSystem;
class CollisionSystem;
class RenderSystem;
class InputSystem;
class ObjectManipulationSystem;

// Define demo scene types
enum class DemoSceneType
{
    Default,
    SphereVsSphere,
    SphereVsAABB,  // Kept this one as the canonical version
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
    PlaneVsSphere
};

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

    // Scene management functions
    void SetupScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader);
    void ClearScene(Registry& registry);
    void SwitchScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType);
    
    // Scene setup helpers for collision demos
    void SetupSphereVsSphereDemo(Registry& registry, const std::shared_ptr<Shader>& shader);
    void SetupAABBVsSphereDemo(Registry& registry, const std::shared_ptr<Shader>& shader);
    void SetupAABBVsAABBDemo(Registry& registry, const std::shared_ptr<Shader>& shader);
    void SetupPointBasedDemos(Registry& registry, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType);
    void SetupRayBasedDemos(Registry& registry, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType);
    void SetupPlaneBasedDemos(Registry& registry, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType);
    
    void UpdateTransforms(Registry& registry);
}