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
        EventSystem::Get().SubscribeToEvent(EventType::KeyPress, [&](const EventData& eventData) {
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
        ResourceHandle meshHandleRhino  = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-3/models/rhino.obj");
        ResourceHandle meshHandleCup    = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-3/models/cup.obj");
        ResourceHandle meshHandleBunny  = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-3/models/bunny.obj");
        ResourceHandle meshHandleCube   = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-3/models/cube.obj");
        ResourceHandle meshHandleGun    = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-3/models/gun.obj");
        ResourceHandle meshHandleCat    = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-3/models/cat.obj");
        ResourceHandle meshHandleArm    = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-3/models/arm.obj");
        ResourceHandle meshHandleStuffed = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-3/models/stuffed.obj");
        
        auto shader = Systems::g_RenderSystem->GetShader();
        
        // Positions for each model so they appear side-by-side
        const glm::vec3 posRhino  (-9.0f, 0.0f, 0.0f);
        const glm::vec3 posCup    (-6.0f, 0.0f, 0.0f);
        const glm::vec3 posBunny  (-3.0f, 0.0f, 0.0f);
        const glm::vec3 posCube   ( 0.0f, 0.0f, 0.0f);
        const glm::vec3 posGun    ( 3.0f, 0.0f, 0.0f);
        const glm::vec3 posCat    ( 6.0f, 0.0f, 0.0f);
        const glm::vec3 posArm    ( 9.0f, 0.0f, 0.0f);
        const glm::vec3 posStuffed (12.0f, 0.0f, 0.0f);
        
        auto createMeshEntity = [&](ModelType type, ResourceHandle handle, const glm::vec3& pos, const glm::vec3& baseScale){
            auto e = registry.Create();
            s_ModelEntities[static_cast<int>(type)] = e;

            // Temporary bounding component just to compute raw extents for normalisation
            BoundingComponent tempBC(handle);
            const Aabb& meshAabb = tempBC.GetAABB();
            float maxExtent = glm::compMax(meshAabb.GetExtents());
            float targetExtent = 0.5f; // We want each model to fit roughly inside unit cube of size 1
            float normaliseScale = (maxExtent > 0.0f) ? targetExtent / maxExtent : 1.0f;

            glm::vec3 finalScale = baseScale * normaliseScale;

            registry.AddComponent<TransformComponent>(e, TransformComponent(pos, glm::vec3(0.0f), finalScale));

            auto meshRenderer = std::make_shared<MeshRenderer>(handle, glm::vec3(0.0f,1.0f,0.0f));

            // Use proper bounding component (with renderables) after scaling decision
            auto boundingComp = BoundingComponent(handle);
            boundingComp.InitializeRenderables(shader);
            registry.AddComponent<BoundingComponent>(e, boundingComp);
            registry.AddComponent<RenderComponent>(e, RenderComponent(meshRenderer));
        };

        // Create all models using helper
        createMeshEntity(ModelType::Rhino,  meshHandleRhino,  posRhino,  glm::vec3(1.0f));
        createMeshEntity(ModelType::Cup,    meshHandleCup,    posCup,    glm::vec3(1.0f));
        createMeshEntity(ModelType::Bunny,  meshHandleBunny,  posBunny,  glm::vec3(1.5f));
        createMeshEntity(ModelType::Cube,    meshHandleCube,    posCube,    glm::vec3(1.0f));
        createMeshEntity(ModelType::Gun,     meshHandleGun,     posGun,     glm::vec3(1.0f));
        createMeshEntity(ModelType::Cat,     meshHandleCat,     posCat,     glm::vec3(1.0f));
        createMeshEntity(ModelType::Arm,     meshHandleArm,     posArm,     glm::vec3(1.0f));
        createMeshEntity(ModelType::Stuffed, meshHandleStuffed, posStuffed, glm::vec3(1.0f));
        
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
            EventSystem::Get().FireEvent(EventType::TransformChanged, entity);
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