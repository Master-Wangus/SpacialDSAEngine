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

#include <glm/gtc/matrix_transform.hpp>

namespace
{
    
    static Registry::Entity CreateSphereEntity(Registry& registry, const glm::vec3& center, float radius, const glm::vec3& color, std::shared_ptr<Shader> shader)
    {
        auto entity = registry.Create();
        
        auto sphereRenderable = std::make_shared<SphereRenderer>(center, radius, color);
        sphereRenderable->Initialize(shader);
        
        registry.AddComponent<TransformComponent>(entity, TransformComponent(center, glm::vec3(0.0f), glm::vec3(radius)));
        registry.AddComponent<RenderComponent>(entity, RenderComponent(sphereRenderable));
        
        // Add a collision component
        registry.AddComponent<CollisionComponent>(entity, CollisionComponent::CreateSphere(center, radius));
        
        return entity;
    }
    
    static Registry::Entity CreateAABBEntity(Registry& registry, const glm::vec3& center, const glm::vec3& halfExtents, const glm::vec3& color, std::shared_ptr<Shader> shader)
    {
        auto entity = registry.Create();
        
        auto cubeRenderable = std::make_shared<CubeRenderer>(center, halfExtents * 2.0f, color);
        cubeRenderable->Initialize(shader);
        
        registry.AddComponent<TransformComponent>(entity, TransformComponent(center, glm::vec3(0.0f), halfExtents * 2.0f));
        registry.AddComponent<RenderComponent>(entity, RenderComponent(cubeRenderable));
        
        // Add a collision component
        registry.AddComponent<CollisionComponent>(entity, CollisionComponent::CreateAABB(center, halfExtents));
        
        return entity;
    }
    
    static Registry::Entity CreateTriangleEntity(Registry& registry, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& color, std::shared_ptr<Shader> shader)
    {
        auto entity = registry.Create();
        
        auto triangleRenderable = std::make_shared<TriangleRenderer>(v0, v1, v2, color);
        triangleRenderable->Initialize(shader);
        
        // Calculate center position for transform
        glm::vec3 center = (v0 + v1 + v2) / 3.0f;
        registry.AddComponent<TransformComponent>(entity, TransformComponent(center, glm::vec3(0.0f), glm::vec3(1.0f)));
        registry.AddComponent<RenderComponent>(entity, RenderComponent(triangleRenderable));
        
        // Add a collision component
        registry.AddComponent<CollisionComponent>(entity, CollisionComponent::CreateTriangle(v0, v1, v2));
        
        return entity;
    }
    
    static Registry::Entity CreatePlaneEntity(Registry& registry, const glm::vec3& normal, float distance, const glm::vec3& color, std::shared_ptr<Shader> shader)
    {
        auto entity = registry.Create();
        
        auto planeRenderable = std::make_shared<PlaneRenderer>(normal, distance, color);
        planeRenderable->Initialize(shader);
        
        // Calculate a position on the plane for transform
        glm::vec3 position = normal * distance;
        registry.AddComponent<TransformComponent>(entity, TransformComponent(position, glm::vec3(0.0f), glm::vec3(1.0f)));
        registry.AddComponent<RenderComponent>(entity, RenderComponent(planeRenderable));
        
        // Add a collision component
        registry.AddComponent<CollisionComponent>(entity, CollisionComponent::CreatePlane(normal, distance));
        
        return entity;
    }
    
    static Registry::Entity CreateRayEntity(Registry& registry, const glm::vec3& origin, const glm::vec3& direction, float length, const glm::vec3& color, std::shared_ptr<Shader> shader)
    {
        auto entity = registry.Create();
        
        auto rayRenderable = std::make_shared<RayRenderer>(origin, direction, color, length);
        rayRenderable->Initialize(shader);
        
        registry.AddComponent<TransformComponent>(entity, TransformComponent(origin, glm::vec3(0.0f), glm::vec3(1.0f)));
        registry.AddComponent<RenderComponent>(entity, RenderComponent(rayRenderable));
        
        // Add a collision component
        registry.AddComponent<CollisionComponent>(entity, CollisionComponent::CreateRay(origin, direction, length));
        
        return entity;
    }
    
    static Registry::Entity CreatePointEntity(Registry& registry, const glm::vec3& position, const glm::vec3& color, std::shared_ptr<Shader> shader)
    {
        auto entity = registry.Create();
        
        auto sphereRenderable = std::make_shared<SphereRenderer>(position, 1.0f, color);
        sphereRenderable->Initialize(shader);
        
        registry.AddComponent<TransformComponent>(entity, TransformComponent(position, glm::vec3(0.0f), glm::vec3(0.01f)));
        registry.AddComponent<RenderComponent>(entity, RenderComponent(sphereRenderable));
        
        // Add a collision component
        registry.AddComponent<CollisionComponent>(entity, CollisionComponent::CreatePoint(position));
        
        return entity;
    }
}

// Define the global system instances
namespace Systems
{
    std::unique_ptr<InputSystem> g_InputSystem = nullptr;
    std::unique_ptr<FPSCameraSystem> g_CameraSystem = nullptr;
    std::unique_ptr<CollisionSystem> g_CollisionSystem = nullptr;
    std::unique_ptr<RenderSystem> g_RenderSystem = nullptr;
    std::unique_ptr<ObjectManipulationSystem> g_ObjectManipulationSystem = nullptr;
    DemoSceneType g_CurrentDemoScene = DemoSceneType::SphereVsSphere;
}

namespace Systems 
{

    void InitializeSystems(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader) 
    {
        // Initialize InputSystem first as other systems will use it
        g_InputSystem = std::make_unique<InputSystem>(registry, window);
        
        g_CameraSystem = std::make_unique<FPSCameraSystem>(registry, window);
        g_CollisionSystem = std::make_unique<CollisionSystem>(registry);
        g_RenderSystem = std::make_unique<RenderSystem>(registry, window, shader);
        g_ObjectManipulationSystem = std::make_unique<ObjectManipulationSystem>(registry, window);
        
        SetupScene(registry, window, shader);
        g_RenderSystem->Initialize();
    }
    
    void SetupScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader) 
    {
        switch (g_CurrentDemoScene)
        {
                
            case DemoSceneType::SphereVsSphere:
                SetupSphereVsSphereDemo(registry, shader);
                break;
                
            case DemoSceneType::SphereVsAABB:
                SetupAABBVsSphereDemo(registry, shader);
                break;
                
            case DemoSceneType::AABBVsAABB:
                SetupAABBVsAABBDemo(registry, shader);
                break;
                
            case DemoSceneType::PointVsSphere:
            case DemoSceneType::PointVsAABB:
            case DemoSceneType::PointVsTriangle:
            case DemoSceneType::PointVsPlane:
                SetupPointBasedDemos(registry, shader, g_CurrentDemoScene);
                break;
                
            case DemoSceneType::RayVsPlane:
            case DemoSceneType::RayVsAABB:
            case DemoSceneType::RayVsSphere:
            case DemoSceneType::RayVsTriangle:
                SetupRayBasedDemos(registry, shader, g_CurrentDemoScene);
                break;
                
            case DemoSceneType::PlaneVsAABB:
            case DemoSceneType::PlaneVsSphere:
                SetupPlaneBasedDemos(registry, shader, g_CurrentDemoScene);
                break;
        }
    }
    
    void UpdateSystems(Registry& registry, Window& window, float deltaTime) 
    {
        // Update input system first
        g_InputSystem->Update(deltaTime);
        
        g_CameraSystem->OnRun(deltaTime);
        
        // Update object manipulation system
        g_ObjectManipulationSystem->Update(deltaTime);
        
        UpdateTransforms(registry);
        
        g_CollisionSystem->UpdateColliders();
        g_CollisionSystem->DetectCollisions();
    }
    
    void RenderSystems(Registry& registry, Window& window) 
    {
        g_RenderSystem->Render();
    }
    
    void ShutdownSystems(Registry& registry) 
    {
        g_RenderSystem->Shutdown();
        
        g_ObjectManipulationSystem.reset();
        g_RenderSystem.reset();
        g_CollisionSystem.reset();
        g_CameraSystem.reset();
        g_InputSystem.reset();
    }
    
    void ClearScene(Registry& registry)
    {
        auto view = registry.GetRegistry().view<RenderComponent>();
        for (auto entity : view)
        {
            registry.Destroy(entity);
        }
    }
    
    void SwitchScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType)
    {
        ClearScene(registry);
        g_CurrentDemoScene = sceneType;
        SetupScene(registry, window, shader);
    }

    void SetupSphereVsSphereDemo(Registry& registry, const std::shared_ptr<Shader>& shader)
    {
        // Create two spheres for collision testing
        CreateSphereEntity(registry, glm::vec3(-1.0f, 0.0f, 0.0f), 1.0f, glm::vec3(1.0f, 0.2f, 0.2f), shader);
        CreateSphereEntity(registry, glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.2f, 0.2f, 1.0f), shader);
    }
    
    void SetupAABBVsSphereDemo(Registry& registry, const std::shared_ptr<Shader>& shader)
    {
        // Used for SphereVsAABB demo (the canonical version - AABBVsSphere was removed)
        // Create a box and a sphere for collision testing
        CreateAABBEntity(registry, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.8f), glm::vec3(0.2f, 0.8f, 0.2f), shader);
        CreateSphereEntity(registry, glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.2f, 0.2f, 1.0f), shader);
    }
    
    void SetupAABBVsAABBDemo(Registry& registry, const std::shared_ptr<Shader>& shader)
    {
        // Create two boxes for collision testing
        CreateAABBEntity(registry, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.8f), glm::vec3(0.8f, 0.2f, 0.2f), shader);
        CreateAABBEntity(registry, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.8f), glm::vec3(0.2f, 0.2f, 0.8f), shader);
    }
    
    void SetupPointBasedDemos(Registry& registry, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType)
    {
        // Point position will be set based on the specific demo
        glm::vec3 pointPosition(0.0f);
        glm::vec3 pointColor(1.0f, 1.0f, 0.0f);
        
        switch (sceneType)
        {
            case DemoSceneType::PointVsSphere:
                pointPosition = glm::vec3(0.0f, 0.0f, 0.0f);
                CreateSphereEntity(registry, glm::vec3(1.5f, 0.0f, 0.0f), 1.0f, glm::vec3(0.2f, 0.2f, 1.0f), shader);
                break;
                
            case DemoSceneType::PointVsAABB:
                pointPosition = glm::vec3(0.0f, 0.0f, 0.0f);
                CreateAABBEntity(registry, glm::vec3(1.5f, 0.0f, 0.0f), glm::vec3(0.8f), glm::vec3(0.2f, 0.8f, 0.2f), shader);
                break;
                
            case DemoSceneType::PointVsTriangle:
            {
                // Define triangle vertices
                glm::vec3 v0(0.0f, 0.0f, 0.0f);
                glm::vec3 v1(2.0f, 0.0f, 0.0f);
                glm::vec3 v2(1.0f, 2.0f, 0.0f);
                
                CreateTriangleEntity(registry, v0, v1, v2, glm::vec3(0.8f, 0.2f, 0.8f), shader);
                break;
            }
                
            case DemoSceneType::PointVsPlane:
            {
                // Define plane parameters
                glm::vec3 planeNormal(0.0f, 1.0f, 0.0f);
                float planeDistance = 0.0f;
                
                CreatePlaneEntity(registry, planeNormal, planeDistance, glm::vec3(0.5f, 0.5f, 0.5f), shader);
                break;
            }
                
            default:
                break;
        }
        
        // Create the point entity with the appropriate position
        auto pointEntity = CreatePointEntity(registry, pointPosition, pointColor, shader);
    }
    
    void SetupRayBasedDemos(Registry& registry, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType)
    {
        // Create a ray for all demos
        auto rayEntity = CreateRayEntity(registry, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 10.0f, glm::vec3(1.0f, 1.0f, 0.0f), shader);
        
        switch (sceneType)
        {
            case DemoSceneType::RayVsPlane:
                // Use a normal facing directly towards the ray (opposite direction of ray)
                // This ensures the ray will intersect the plane when pointing in the positive X direction
                CreatePlaneEntity(registry, glm::vec3(-1.0f, 0.0f, 0.0f), 3.0f, glm::vec3(0.5f, 0.5f, 0.5f), shader);
                break;
                
            case DemoSceneType::RayVsAABB:
                CreateAABBEntity(registry, glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0.8f), glm::vec3(0.2f, 0.8f, 0.2f), shader);
                break;
                
            case DemoSceneType::RayVsSphere:
                CreateSphereEntity(registry, glm::vec3(3.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.2f, 0.2f, 1.0f), shader);
                break;
                
            case DemoSceneType::RayVsTriangle:
                CreateTriangleEntity(registry, 
                    glm::vec3(2.0f, -1.0f, -1.0f), 
                    glm::vec3(2.0f, 1.0f, -1.0f), 
                    glm::vec3(2.0f, 0.0f, 1.0f), 
                    glm::vec3(0.8f, 0.2f, 0.8f), shader);
                break;
                
            default:
                break;
        }
    }
    
    void SetupPlaneBasedDemos(Registry& registry, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType)
    {
        // Create a plane for all demos
        auto planeEntity = CreatePlaneEntity(registry, glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(0.5f, 0.5f, 0.5f), shader);
        
        switch (sceneType)
        {
            case DemoSceneType::PlaneVsAABB:
                CreateAABBEntity(registry, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.8f), glm::vec3(0.2f, 0.8f, 0.2f), shader);
                break;
                
            case DemoSceneType::PlaneVsSphere:
                CreateSphereEntity(registry, glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, glm::vec3(0.2f, 0.2f, 1.0f), shader);
                break;
                
            default:
                break;
        }
    }

    void UpdateTransforms(Registry& registry)
    {
        auto view = registry.GetRegistry().view<TransformComponent>();

        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            transform.UpdateModelMatrix();
        }
    }
}
