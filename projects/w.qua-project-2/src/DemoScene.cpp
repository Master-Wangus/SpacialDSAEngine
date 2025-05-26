/**
 * @class DemoScene
 * @brief Demonstration scene with multiple 3D objects and interaction.
 *
 * This class sets up a 3D scene with various primitive objects, lighting, and
 * interactive components to showcase the engine's capabilities.
 */

#include "DemoScene.hpp"
#include "Components.hpp"
#include "Registry.hpp"
#include "Systems.hpp"
#include "Window.hpp"
#include "Shader.hpp"
#include "RenderSystem.hpp"
#include "SphereRenderer.hpp"
#include "CubeRenderer.hpp"
#include "TriangleRenderer.hpp"
#include "PlaneRenderer.hpp"
#include "RayRenderer.hpp"
#include "MeshRenderer.hpp"
#include "ResourceSystem.hpp"

namespace
{
    // Helper functions for creating various entities
    Registry::Entity CreateSphereEntity(Registry& registry, const glm::vec3& center, float radius, const glm::vec3& color, std::shared_ptr<Shader> shader)
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
    
    Registry::Entity CreateAABBEntity(Registry& registry, const glm::vec3& center, const glm::vec3& halfExtents, const glm::vec3& color, std::shared_ptr<Shader> shader)
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
    
    Registry::Entity CreateTriangleEntity(Registry& registry, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& color, std::shared_ptr<Shader> shader)
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
    
    Registry::Entity CreatePlaneEntity(Registry& registry, const glm::vec3& normal, float distance, const glm::vec3& color, std::shared_ptr<Shader> shader)
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
    
    Registry::Entity CreateRayEntity(Registry& registry, const glm::vec3& origin, const glm::vec3& direction, float length, const glm::vec3& color, std::shared_ptr<Shader> shader)
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
    
    Registry::Entity CreatePointEntity(Registry& registry, const glm::vec3& position, const glm::vec3& color, std::shared_ptr<Shader> shader)
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

namespace DemoScene 
{
    void SetupScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType) 
    {
        switch (sceneType)
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
                SetupPointBasedDemos(registry, shader, sceneType);
                break;
                
            case DemoSceneType::RayVsPlane:
            case DemoSceneType::RayVsAABB:
            case DemoSceneType::RayVsSphere:
            case DemoSceneType::RayVsTriangle:
                SetupRayBasedDemos(registry, shader, sceneType);
                break;
                
            case DemoSceneType::PlaneVsAABB:
            case DemoSceneType::PlaneVsSphere:
                SetupPlaneBasedDemos(registry, shader, sceneType);
                break;
                
            case DemoSceneType::MeshResource:
                SetupMeshResourceDemo(registry, shader);
                break;
        }
    }
    
    void ClearScene(Registry& registry)
    {
        auto view = registry.View<TransformComponent, RenderComponent>();
        for (auto entity : view)
        {
            // Skip camera entity
            if (registry.HasComponent<CameraComponent>(entity))
                continue;
                
            // Skip light visualization entity
            if (Systems::g_RenderSystem && entity == Systems::g_RenderSystem->GetLightVisualizationEntity())
                continue;
                
            registry.Destroy(entity);
        }
    }
    
    void SwitchScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType)
    {
        ClearScene(registry);
        SetupScene(registry, window, shader, sceneType);
    }
    
    void SetupSphereVsSphereDemo(Registry& registry, const std::shared_ptr<Shader>& shader)
    {
        // Create two spheres for collision testing
        CreateSphereEntity(registry, glm::vec3(-1.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.0f, 1.0f, 0.0f), shader); // Green
        CreateSphereEntity(registry, glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.0f, 0.0f, 1.0f), shader);  // Blue
    }
    
    void SetupAABBVsSphereDemo(Registry& registry, const std::shared_ptr<Shader>& shader)
    {
        // Create a sphere and an AABB for collision testing
        CreateSphereEntity(registry, glm::vec3(-1.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.0f, 1.0f, 0.0f), shader); // Green
        CreateAABBEntity(registry, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.5f), glm::vec3(0.0f, 0.0f, 1.0f), shader); // Blue
    }
    
    void SetupAABBVsAABBDemo(Registry& registry, const std::shared_ptr<Shader>& shader)
    {
        // Create two AABBs for collision testing
        CreateAABBEntity(registry, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.5f), glm::vec3(0.0f, 1.0f, 0.0f), shader); // Green
        CreateAABBEntity(registry, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.5f), glm::vec3(0.0f, 0.0f, 1.0f), shader);  // Blue
    }
    
    void SetupPointBasedDemos(Registry& registry, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType)
    {
        auto pointEntity = CreatePointEntity(registry, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), shader);
        
        // Create the appropriate test shape based on demo type
        switch (sceneType)
        {
            case DemoSceneType::PointVsSphere:
            {
                CreateSphereEntity(registry, glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.0f, 0.4f, 1.0f), shader);
                break;
            }
            
            case DemoSceneType::PointVsAABB:
            {
                CreateAABBEntity(registry, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.5f), glm::vec3(0.0f, 0.4f, 1.0f), shader);
                break;
            }
            
            case DemoSceneType::PointVsTriangle:
            {
                glm::vec3 v0(0.0f, 0.0f, 0.0f);
                glm::vec3 v1(1.0f, 0.0f, 0.0f);
                glm::vec3 v2(0.5f, 1.0f, 0.0f);
                CreateTriangleEntity(registry, v0, v1, v2, glm::vec3(0.0f, 0.4f, 1.0f), shader);
                break;
            }
            
            case DemoSceneType::PointVsPlane:
            {
                CreatePlaneEntity(registry, glm::vec3(0.0f, 1.0f, 0.0f), 0.5f, glm::vec3(0.0f, 0.4f, 1.0f), shader);
                break;
            }
        }
    }
    
    void SetupRayBasedDemos(Registry& registry, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType)
    {
        auto rayEntity = CreateRayEntity(
            registry, 
            glm::vec3(0.0f, 0.0f, -5.0f), // Origin behind camera
            glm::vec3(0.0f, 0.0f, 1.0f),  // Direction forward
            10.0f,                         // Length
            glm::vec3(1.0f, 1.0f, 0.0f),  // Yellow color
            shader
        );
        
        switch (sceneType)
        {
            case DemoSceneType::RayVsPlane:
            {
                // Plane in front of the ray
                CreatePlaneEntity(registry, glm::vec3(0.0f, 0.0f, -1.0f), -2.0f, glm::vec3(0.0f, 0.4f, 1.0f), shader);
                break;
            }
            
            case DemoSceneType::RayVsAABB:
            {
                // AABB in front of the ray
                CreateAABBEntity(registry, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.5f), glm::vec3(0.0f, 0.4f, 1.0f), shader);
                break;
            }
            
            case DemoSceneType::RayVsSphere:
            {
                // Sphere in front of the ray
                CreateSphereEntity(registry, glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.0f, 0.4f, 1.0f), shader);
                break;
            }
            
            case DemoSceneType::RayVsTriangle:
            {
                // Triangle in front of the ray
                glm::vec3 v0(-1.0f, -1.0f, 0.0f);
                glm::vec3 v1(1.0f, -1.0f, 0.0f);
                glm::vec3 v2(0.0f, 1.0f, 0.0f);
                CreateTriangleEntity(registry, v0, v1, v2, glm::vec3(0.0f, 0.4f, 1.0f), shader);
                break;
            }
        }
    }
    
    void SetupPlaneBasedDemos(Registry& registry, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType)
    {
        // Create a plane entity
        auto planeEntity = CreatePlaneEntity(
            registry,
            glm::vec3(0.0f, 1.0f, 0.0f),  // Normal pointing up
            0.0f,                          // Distance from origin
            glm::vec3(0.0f, 0.4f, 1.0f),   // Blue color
            shader
        );
        
        // Create the appropriate test shape based on demo type
        switch (sceneType)
        {
            case DemoSceneType::PlaneVsAABB:
            {
                // AABB near the plane
                CreateAABBEntity(registry, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.5f), glm::vec3(0.0f, 0.4f, 1.0f), shader);
                break;
            }
            
            case DemoSceneType::PlaneVsSphere:
            {
                // Sphere near the plane
                CreateSphereEntity(registry, glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, glm::vec3(0.0f, 0.4f, 1.0f), shader);
                break;
            }
        }
    }
    
    void UpdateTransforms(Registry& registry)
    {
        auto view = registry.View<TransformComponent, CollisionComponent>();
        for (auto entity : view)
        {
            auto& transform = registry.GetComponent<TransformComponent>(entity);
            auto& collider = registry.GetComponent<CollisionComponent>(entity);
            
            // Update the collider transform based on the entity transform
            collider.UpdateTransform(transform.m_Position, transform.m_Scale);
        }
    }

    void SetupMeshResourceDemo(Registry& registry, const std::shared_ptr<Shader>& shader)
    {
        // Create a floor plane
        auto floor = registry.Create();
        registry.AddComponent<TransformComponent>(floor, 
            TransformComponent(glm::vec3(0.0f, -1.0f, 0.0f), 
                              glm::vec3(0.0f), 
                              glm::vec3(10.0f, 0.1f, 10.0f)));
        
        auto floorRenderer = std::make_shared<CubeRenderer>(
            glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
        floorRenderer->Initialize(shader);
        registry.AddComponent<RenderComponent>(floor, RenderComponent(floorRenderer));
        
        // Create a mesh entity (replace with the path to your .obj file)
        const std::string meshPath = "../projects/w.qua-project-2/models/teddy.obj";  
        
        // Load the mesh and get a handle from the ResourceSystem
        ResourceHandle meshHandle = ResourceSystem::GetInstance().LoadMesh(meshPath);
        
        
        auto meshEntity = registry.Create();
        registry.AddComponent<TransformComponent>(meshEntity,
            TransformComponent(glm::vec3(0.0f, 1.0f, 0.0f),
                              glm::vec3(0.0f),
                              glm::vec3(1.0f)));
        
        // Create a mesh renderer using the resource handle
        auto meshRenderer = std::make_shared<MeshRenderer>(meshHandle, glm::vec3(0.0f, 1.0f, 0.0f));
        meshRenderer->Initialize(shader);
        registry.AddComponent<RenderComponent>(meshEntity, RenderComponent(meshRenderer));
        
        // Add a collision component for the mesh
        auto collisionComp = CollisionComponent::CreateSphere(glm::vec3(0.0f), 1.0f);
        registry.AddComponent<CollisionComponent>(meshEntity, collisionComp);
    }
} 