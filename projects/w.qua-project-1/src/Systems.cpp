#include "Systems.hpp"
#include "Components.hpp"
#include "Shader.hpp"
#include "Intersection.hpp"
#include "Window.hpp"
#include "Cube.hpp"
#include "Registry.hpp"
#include "FPSCameraSystem.hpp"
#include "CollisionSystem.hpp"
#include "RenderSystem.hpp"
#include "InputSystem.hpp"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

namespace
{
    static Registry::Entity CreateCubeEntity(Registry& registry, const glm::vec3& position, float size, const glm::vec3& color, std::shared_ptr<Shader> shader)
    {
        auto entity = registry.Create();

        auto cubeRenderable = std::make_shared<Cube>(color, size);
        
        Material material;
        material.m_AmbientColor = color;
        material.m_DiffuseColor = color;
        material.m_SpecularColor = color;
        material.m_Shininess = 32.0f;
        cubeRenderable->SetMaterial(material);

        registry.AddComponent<TransformComponent>(entity, TransformComponent(position, glm::vec3(0.0f), glm::vec3(size)));
        registry.AddComponent<RenderComponent>(entity, RenderComponent(cubeRenderable));

        registry.AddComponent<AABBComponent>(entity, AABBComponent(position, glm::vec3(size * 0.5f)));
        registry.AddComponent<BoundingSphereComponent>(entity, BoundingSphereComponent(position, size * 0.866025f));

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
        
        SetupScene(registry, window, shader);
        g_RenderSystem->Initialize();
    }
    
    void SetupScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader) 
    {
        CreateCubes(registry, shader);
    }
    
    void UpdateSystems(Registry& registry, Window& window, float deltaTime) 
    {
        // Update input system first
        g_InputSystem->Update(deltaTime);
        
        g_CameraSystem->OnRun(deltaTime);
        
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
        
        g_RenderSystem.reset();
        g_CollisionSystem.reset();
        g_CameraSystem.reset();
        g_InputSystem.reset();
    }
    
    
    void CreateCubes(Registry& registry, const std::shared_ptr<Shader>& shader)
    {
        // Create center cube
        CreateCubeEntity(registry, glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, glm::vec3(1.0f, 0.5f, 0.31f), shader);
        
        // Create surrounding cubes
        CreateCubeEntity(registry, glm::vec3(2.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.31f, 0.5f, 1.0f), shader);
        CreateCubeEntity(registry, glm::vec3(-2.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.5f, 1.0f, 0.31f), shader);
        CreateCubeEntity(registry, glm::vec3(0.0f, 2.0f, 0.0f), 1.0f, glm::vec3(1.0f, 0.31f, 0.5f), shader);
        CreateCubeEntity(registry, glm::vec3(0.0f, -2.0f, 0.0f), 1.0f, glm::vec3(0.31f, 1.0f, 0.5f), shader);
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
