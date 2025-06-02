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

namespace DemoScene 
{
    void SetupScene(Registry& registry, Window& window, DemoSceneType sceneType) 
    {
        SetupMeshScene(registry);
    }
    
    void ClearScene(Registry& registry)
    {
        auto view = registry.View<TransformComponent, RenderComponent>();
        for (auto entity : view)
        {
            if (registry.HasComponent<CameraComponent>(entity))
                continue;
                
            if (Systems::g_RenderSystem && entity == Systems::g_RenderSystem->GetLightVisualizationEntity())
                continue;
                
            registry.Destroy(entity);
        }
    }

    void ResetScene(Registry& registry, Window& window)
    {
        ClearScene(registry);        
        SetupScene(registry, window, Systems::g_CurrentDemoScene);
    }

    void SetupMeshScene(Registry& registry)
    {
        ResourceHandle meshHandleRhino = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-2/models/rhino.obj");
        ResourceHandle meshHandleCup = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-2/models/cup.obj");
        
        // Get the shader from the render system for initializing renderables
        auto shader = Systems::g_RenderSystem->GetShader();
        
        // Rhino
        auto meshEntityRhino = registry.Create();
        registry.AddComponent<TransformComponent>(meshEntityRhino,
            TransformComponent(glm::vec3(0.0f, 0.0f, 0.0f),
                              glm::vec3(0.0f),
                              glm::vec3(1.0f)));
        
        auto meshRendererRhino = std::make_shared<MeshRenderer>(meshHandleRhino, glm::vec3(0.0f, 1.0f, 0.0f), true);

        // Create bounding component for rhino
        auto boundingComponentRhino = BoundingComponent(meshHandleRhino);
        boundingComponentRhino.InitializeRenderables(shader);
        registry.AddComponent<BoundingComponent>(meshEntityRhino, boundingComponentRhino);
        
        // Create render component with main renderable only
        RenderComponent renderCompRhino(meshRendererRhino);
        registry.AddComponent<RenderComponent>(meshEntityRhino, renderCompRhino);

        // Cup
        auto meshEntityCup = registry.Create();
        registry.AddComponent<TransformComponent>(meshEntityCup,
            TransformComponent(glm::vec3(0.0f, 0.0f, 15.0f),
                glm::vec3(0.0f),
                glm::vec3(1.0f)));
        
        auto meshRendererCup = std::make_shared<MeshRenderer>(meshHandleCup, glm::vec3(0.0f, 1.0f, 0.0f), true);
        
        // Create bounding component for cup
        auto boundingComponentCup = BoundingComponent(meshHandleCup);
        boundingComponentCup.InitializeRenderables(shader);
        registry.AddComponent<BoundingComponent>(meshEntityCup, boundingComponentCup);
        
        // Create render component with main renderable only
        RenderComponent renderCompCup(meshRendererCup);
        registry.AddComponent<RenderComponent>(meshEntityCup, renderCompCup);
    }
} 