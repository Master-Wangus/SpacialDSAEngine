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
#include "EventSystem.hpp"
#include "DemoScene.hpp"
#include "PickingSystem.hpp"

namespace Systems
{
    std::unique_ptr<InputSystem> g_InputSystem = nullptr;
    std::unique_ptr<CameraSystem> g_CameraSystem = nullptr;
    std::unique_ptr<RenderSystem> g_RenderSystem = nullptr;
    std::unique_ptr<PickingSystem> g_PickingSystem = nullptr;
    DemoSceneType g_CurrentDemoScene = DemoSceneType::MeshScene;

    void InitializeSystems(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader) 
    {
        EventSystem::Get().Initialize();        
        g_InputSystem = std::make_unique<InputSystem>(registry, window);
        g_CameraSystem = std::make_unique<CameraSystem>(registry, window);
        g_RenderSystem = std::make_unique<RenderSystem>(registry, window, shader);
        g_PickingSystem = std::make_unique<PickingSystem>(registry, window);        
        g_RenderSystem->SetCameraSystem(g_CameraSystem.get());
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
        g_RenderSystem.reset();
        g_CameraSystem.reset();
        g_InputSystem.reset();
        g_PickingSystem.reset();
        EventSystem::Get().Shutdown();
    }

    void ResetCurrentScene(Registry& registry, Window& window)
    {
        if (g_RenderSystem)
        {
            DemoScene::ResetScene(registry, window);
            
            EventSystem::Get().FireEvent(EventType::SceneReset);
        }
    }
}
