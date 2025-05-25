#include "Systems.hpp"
#include "Components.hpp"
#include "Shader.hpp"
#include "Intersection.hpp"
#include "Window.hpp"
#include "Registry.hpp"
#include "FPSCameraSystem.hpp"
#include "CollisionSystem.hpp"
#include "RenderSystem.hpp"
#include "InputSystem.hpp"
#include "ObjectManipulationSystem.hpp"
#include "SphereRenderer.hpp"
#include "CubeRenderer.hpp"
#include "TriangleRenderer.hpp"
#include "PlaneRenderer.hpp"
#include "RayRenderer.hpp"
#include "DemoScene.hpp"

#include <glm/gtc/matrix_transform.hpp>


namespace Systems
{
    std::unique_ptr<InputSystem> g_InputSystem = nullptr;
    std::unique_ptr<FPSCameraSystem> g_CameraSystem = nullptr;
    std::unique_ptr<CollisionSystem> g_CollisionSystem = nullptr;
    std::unique_ptr<RenderSystem> g_RenderSystem = nullptr;
    std::unique_ptr<ObjectManipulationSystem> g_ObjectManipulationSystem = nullptr;
    DemoSceneType g_CurrentDemoScene = DemoSceneType::SphereVsSphere;

    void InitializeSystems(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader) 
    {
        // Initialize InputSystem first as other systems will use it
        g_InputSystem = std::make_unique<InputSystem>(registry, window);
        
        g_CameraSystem = std::make_unique<FPSCameraSystem>(registry, window);
        g_CollisionSystem = std::make_unique<CollisionSystem>(registry);
        g_RenderSystem = std::make_unique<RenderSystem>(registry, window, shader);
        g_ObjectManipulationSystem = std::make_unique<ObjectManipulationSystem>(registry, window);
        
        DemoScene::SetupScene(registry, window, shader, g_CurrentDemoScene);
        g_RenderSystem->Initialize();
    }
    
    void UpdateSystems(Registry& registry, Window& window, float deltaTime) 
    {
        g_InputSystem->Update(deltaTime);
        g_CameraSystem->OnRun(deltaTime);
        g_ObjectManipulationSystem->Update(deltaTime);
        DemoScene::UpdateTransforms(registry);
        g_CollisionSystem->DetectCollisions();
        g_ObjectManipulationSystem->UpdateCollisionColors();
    }
    
    void RenderSystems(Registry& registry, Window& window) 
    {
        g_RenderSystem->Render();
    }
    
    void ShutdownSystems(Registry& registry) 
    {
        // Destroy systems in reverse order of creation
        g_ObjectManipulationSystem.reset();
        g_RenderSystem.reset();
        g_CollisionSystem.reset();
        g_CameraSystem.reset();
        g_InputSystem.reset();
    }
}
