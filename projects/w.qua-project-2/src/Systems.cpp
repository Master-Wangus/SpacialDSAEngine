/**
 * @file Systems.cpp
 * @brief Implementation of the systems registry and management.
 *
 * This file contains the implementation of system registration, initialization,
 * update, and lifecycle management for the entity-component-system architecture.
 */

#include "Systems.hpp"
#include "Components.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include "Registry.hpp"
#include "CameraSystem.hpp"
#include "RenderSystem.hpp"
#include "InputSystem.hpp"
#include "SphereRenderer.hpp"
#include "CubeRenderer.hpp"
#include "TriangleRenderer.hpp"
#include "PlaneRenderer.hpp"
#include "RayRenderer.hpp"
#include "DemoScene.hpp"



namespace Systems
{
    std::unique_ptr<InputSystem> g_InputSystem = nullptr;
    std::unique_ptr<CameraSystem> g_CameraSystem = nullptr;
    std::unique_ptr<RenderSystem> g_RenderSystem = nullptr;
    DemoSceneType g_CurrentDemoScene = DemoSceneType::MeshScene;

    void InitializeSystems(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader) 
    {
        // Initialize InputSystem first as other systems will use it
        g_InputSystem = std::make_unique<InputSystem>(registry, window);
        
        g_CameraSystem = std::make_unique<CameraSystem>(registry, window);
        g_RenderSystem = std::make_unique<RenderSystem>(registry, window, shader);
        
        DemoScene::SetupScene(registry, window, g_CurrentDemoScene);
        g_RenderSystem->Initialize();
    }
    
    void UpdateSystems(Registry& registry, Window& window, float deltaTime) 
    {
        g_InputSystem->Update(deltaTime);
        g_CameraSystem->OnRun(deltaTime);
    }
    
    void RenderSystems(Registry& registry, Window& window) 
    {
        g_RenderSystem->Render();
    }
    
    void ShutdownSystems(Registry& registry) 
    {
        // Destroy systems in reverse order of creation
        g_RenderSystem.reset();
        g_CameraSystem.reset();
        g_InputSystem.reset();
    }

    void ResetCurrentScene(Registry& registry, Window& window)
    {
        if (g_RenderSystem)
        {
            DemoScene::ResetScene(registry, window);
        }
    }
}
