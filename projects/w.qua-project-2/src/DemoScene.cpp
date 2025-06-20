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
#include "InputSystem.hpp"
#include "EventSystem.hpp"
#include "Keybinds.hpp"

namespace DemoScene 
{
    // Store the current model being displayed
    static ModelType s_CurrentModel = ModelType::Rhino;
    
    // Entities for each model type
    static Registry::Entity s_ModelEntities[static_cast<int>(ModelType::Count)];

    void SetupScene(Registry& registry, Window& window, DemoSceneType sceneType) 
    {
        SetupMeshScene(registry);
        
        // Setup scene reset shortcut using EventSystem
        SUBSCRIBE_TO_EVENT(EventType::KeyPress, [&](const EventData& eventData) {
            // Check if the event data contains an integer (key code)
            if (auto keyCode = std::get_if<int>(&eventData)) {
                if (*keyCode == Keybinds::KEY_R) {
                    ResetScene(registry, window);
                    std::cout << "Scene reset!" << std::endl;
                }
            }
        });
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
        
        // Reset entity references
        for (int i = 0; i < static_cast<int>(ModelType::Count); ++i) {
            s_ModelEntities[i] = entt::null;
        }
    }

    void ResetScene(Registry& registry, Window& window)
    {
        ClearScene(registry);        
        SetupScene(registry, window, Systems::g_CurrentDemoScene);
    }

    void SetupMeshScene(Registry& registry)
    {
        // Load all meshes
        ResourceHandle meshHandleRhino = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-2/models/rhino.obj");
        ResourceHandle meshHandleCup = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-2/models/cup.obj");
        ResourceHandle meshHandleBunny = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-2/models/bunny.obj");
        
        auto shader = Systems::g_RenderSystem->GetShader();
        
        // Positions for each model so they appear side-by-side
        const glm::vec3 posRhino (-3.0f, 0.0f, 0.0f);
        const glm::vec3 posCup   ( 0.0f, 0.0f, 0.0f);
        const glm::vec3 posBunny ( 3.0f, 0.0f, 0.0f);
        const glm::vec3 posCube  ( 6.0f, 0.0f, 0.0f);
        
        // Rhino
        auto meshEntityRhino = registry.Create();
        s_ModelEntities[static_cast<int>(ModelType::Rhino)] = meshEntityRhino;
        registry.AddComponent<TransformComponent>(meshEntityRhino,
            TransformComponent(posRhino,
                              glm::vec3(0.0f),
                              glm::vec3(1.0f)));
        
        auto meshRendererRhino = std::make_shared<MeshRenderer>(meshHandleRhino, glm::vec3(0.0f, 1.0f, 0.0f));

        // Create bounding component for rhino
        auto boundingComponentRhino = BoundingComponent(meshHandleRhino);
        boundingComponentRhino.InitializeRenderables(shader);
        registry.AddComponent<BoundingComponent>(meshEntityRhino, boundingComponentRhino);
        
        // Create render component with main renderable only
        RenderComponent renderCompRhino(meshRendererRhino);
        registry.AddComponent<RenderComponent>(meshEntityRhino, renderCompRhino);

        // Cup
        auto meshEntityCup = registry.Create();
        s_ModelEntities[static_cast<int>(ModelType::Cup)] = meshEntityCup;
        registry.AddComponent<TransformComponent>(meshEntityCup,
            TransformComponent(posCup,
                glm::vec3(0.0f),
                glm::vec3(1.0f)));
        
        auto meshRendererCup = std::make_shared<MeshRenderer>(meshHandleCup, glm::vec3(0.0f, 1.0f, 0.0f));
        
        // Create bounding component for cup
        auto boundingComponentCup = BoundingComponent(meshHandleCup);
        boundingComponentCup.InitializeRenderables(shader);
        registry.AddComponent<BoundingComponent>(meshEntityCup, boundingComponentCup);
        
        // Create render component with main renderable only
        RenderComponent renderCompCup(meshRendererCup);
        registry.AddComponent<RenderComponent>(meshEntityCup, renderCompCup);
        
        // Bunny
        auto meshEntityBunny = registry.Create();
        s_ModelEntities[static_cast<int>(ModelType::Bunny)] = meshEntityBunny;
        registry.AddComponent<TransformComponent>(meshEntityBunny,
            TransformComponent(posBunny,
                glm::vec3(0.0f),
                glm::vec3(1.5f)));  // Scaled up for better visibility
        
        auto meshRendererBunny = std::make_shared<MeshRenderer>(meshHandleBunny, glm::vec3(0.0f, 1.0f, 0.0f));
        
        // Create bounding component for bunny
        auto boundingComponentBunny = BoundingComponent(meshHandleBunny);
        boundingComponentBunny.InitializeRenderables(shader);
        registry.AddComponent<BoundingComponent>(meshEntityBunny, boundingComponentBunny);
        
        // Create render component for bunny
        RenderComponent renderCompBunny(meshRendererBunny);
        registry.AddComponent<RenderComponent>(meshEntityBunny, renderCompBunny);
        
        // Simple Cube
        auto cubeEntity = registry.Create();
        s_ModelEntities[static_cast<int>(ModelType::Cube)] = cubeEntity;
        registry.AddComponent<TransformComponent>(cubeEntity,
            TransformComponent(posCube,
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(1.0f)));
        
        // Create cube renderer
        auto cubeRenderer = std::make_shared<CubeRenderer>(
            glm::vec3(0.0f),          // Center (will be positioned by transform)
            glm::vec3(1.0f),          // Size 
            glm::vec3(0.0f, 1.0f, 0.0f) // Green color (consistent with other models)
        );
        cubeRenderer->Initialize(shader);
        
        // Create render component for cube
        RenderComponent renderCompCube(cubeRenderer);
        registry.AddComponent<RenderComponent>(cubeEntity, renderCompCube);
        
        // Create vertices for the cube's bounding volumes
        std::vector<Vertex> cubeVertices;
        // 8 corners of a unit cube
        glm::vec3 positions[8] = {
            glm::vec3(-0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f, 0.5f, -0.5f),
            glm::vec3(-0.5f, 0.5f, -0.5f),
            glm::vec3(-0.5f, -0.5f, 0.5f),
            glm::vec3(0.5f, -0.5f, 0.5f),
            glm::vec3(0.5f, 0.5f, 0.5f),
            glm::vec3(-0.5f, 0.5f, 0.5f)
        };
        
        // Create Vertex objects for each corner
        for (int i = 0; i < 8; i++) {
            Vertex v;
            v.m_Position = positions[i];
            v.m_Color = glm::vec3(0.0f, 1.0f, 0.0f);  // Green color (consistent with other models)
            v.m_Normal = glm::normalize(positions[i]); // Approximate normal
            v.m_UV = glm::vec2(0.0f);
            cubeVertices.push_back(v);
        }
        
        // Create bounding component for cube
        BoundingComponent boundingComponentCube;
        
        // Manually compute AABB
        Vertex min, max;
        CreateAabbBruteForce(cubeVertices.data(), cubeVertices.size(), &min, &max);
        boundingComponentCube.m_AABB = Aabb(min.m_Position, max.m_Position);
        
        // Manually compute spheres
        Vertex center;
        float radius;
        
        // Ritter sphere
        CreateSphereRitters(cubeVertices.data(), cubeVertices.size(), &center, &radius);
        boundingComponentCube.m_RitterSphere = Sphere(center.m_Position, radius);
        
        // Larsson sphere
        CreateSphereIterative(cubeVertices.data(), cubeVertices.size(), 2, 0.5f, &center, &radius);
        boundingComponentCube.m_LarssonSphere = Sphere(center.m_Position, radius);
        
        // PCA sphere
        CreateSpherePCA(cubeVertices.data(), cubeVertices.size(), &center, &radius);
        boundingComponentCube.m_PCASphere = Sphere(center.m_Position, radius);
        
        // PCA OBB
        glm::vec3 obbCenter;
        glm::vec3 obbAxes[3];
        glm::vec3 obbHalfExtents;
        CreateObbPCA(cubeVertices.data(), cubeVertices.size(), &obbCenter, obbAxes, &obbHalfExtents);
        boundingComponentCube.m_OBB = Obb(obbCenter, obbAxes, obbHalfExtents);
        
        // Initialize renderables
        boundingComponentCube.InitializeRenderables(shader);
        registry.AddComponent<BoundingComponent>(cubeEntity, boundingComponentCube);
        
        // Ensure all models are visible by default
        for (int i = 0; i < static_cast<int>(ModelType::Count); ++i) {
            auto ent = s_ModelEntities[i];
            if (ent != entt::null && registry.HasComponent<RenderComponent>(ent)) {
                registry.GetComponent<RenderComponent>(ent).m_IsVisible = true;
            }
        }
    }
    
    // Implementation of model switching functions
    void SwitchToModel(Registry& registry, ModelType modelType)
    {
        s_CurrentModel = modelType;
        
        // Hide all models first
        for (int i = 0; i < static_cast<int>(ModelType::Count); ++i) {
            auto entity = s_ModelEntities[i];
            if (entity != entt::null)
            {
                auto& renderComp = registry.GetComponent<RenderComponent>(entity);
                renderComp.m_IsVisible = false;
            }
        }
        
        // Show only the selected model
        auto selectedEntity = s_ModelEntities[static_cast<int>(modelType)];
        if (selectedEntity != entt::null)
        {
            auto& renderComp = registry.GetComponent<RenderComponent>(selectedEntity);
            renderComp.m_IsVisible = true;
        }
    }
    
    ModelType GetCurrentModel()
    {
        return s_CurrentModel;
    }
    
    void CycleToNextModel(Registry& registry)
    {
        int nextModelIndex = (static_cast<int>(s_CurrentModel) + 1) % static_cast<int>(ModelType::Count);
        SwitchToModel(registry, static_cast<ModelType>(nextModelIndex));
    }
    
    void CycleToPreviousModel(Registry& registry)
    {
        int currentIndex = static_cast<int>(s_CurrentModel);
        currentIndex = (currentIndex - 1 + static_cast<int>(ModelType::Count)) % static_cast<int>(ModelType::Count);
        SwitchToModel(registry, static_cast<ModelType>(currentIndex));
    }

    // ============================
    //  Scaling helper functions
    // ============================

    void SetModelScale(Registry& registry, ModelType modelType, float scale)
    {
        auto entity = s_ModelEntities[static_cast<int>(modelType)];
        if (entity != entt::null && registry.HasComponent<TransformComponent>(entity))
        {
            auto &transform = registry.GetComponent<TransformComponent>(entity);
            transform.m_Scale = glm::vec3(scale);
            transform.UpdateModelMatrix();
        }
    }

    float GetModelScale(Registry& registry, ModelType modelType)
    {
        auto entity = s_ModelEntities[static_cast<int>(modelType)];
        if (entity != entt::null && registry.HasComponent<TransformComponent>(entity))
        {
            return registry.GetComponent<TransformComponent>(entity).m_Scale.x;
        }
        return 1.0f;
    }

    void SetGlobalScale(Registry& registry, float scale)
    {
        for (int i = 0; i < static_cast<int>(ModelType::Count); ++i)
        {
            SetModelScale(registry, static_cast<ModelType>(i), scale);
        }
    }
} 