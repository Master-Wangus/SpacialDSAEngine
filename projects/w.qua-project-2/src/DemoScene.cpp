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
    void SetupScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader, DemoSceneType sceneType) 
    {
        SetupMeshScene(registry, shader);
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

    void SetupMeshScene(Registry& registry, const std::shared_ptr<Shader>& shader)
    {
                
        ResourceHandle meshHandleRhino = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-2/models/rhino.obj");
        ResourceHandle meshHandleCup = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-2/models/cup.obj");
        
        // Rhino
        auto meshEntityRhino = registry.Create();
        registry.AddComponent<TransformComponent>(meshEntityRhino,
            TransformComponent(glm::vec3(0.0f, 0.0f, 0.0f),
                              glm::vec3(0.0f),
                              glm::vec3(0.4f)));
        
        auto meshRendererRhino = std::make_shared<MeshRenderer>(meshHandleRhino, glm::vec3(0.0f, 1.0f, 0.0f));
        meshRendererRhino->Initialize(shader);

        registry.AddComponent<RenderComponent>(meshEntityRhino, RenderComponent(meshRendererRhino));

        // Cup
        auto meshEntityCup = registry.Create();
        registry.AddComponent<TransformComponent>(meshEntityCup,
            TransformComponent(glm::vec3(0.0f, 0.0f, -5.0f),
                glm::vec3(0.0f),
                glm::vec3(10.0f)));
        auto meshRendererCup = std::make_shared<MeshRenderer>(meshHandleCup, glm::vec3(0.0f, 1.0f, 0.0f));
        meshRendererCup->Initialize(shader);
        registry.AddComponent<RenderComponent>(meshEntityCup, RenderComponent(meshRendererCup));
    }
} 