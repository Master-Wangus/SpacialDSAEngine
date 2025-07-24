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
    static std::vector<Registry::Entity> s_SectionEntities[static_cast<int>(SectionId::Count)];

    static const glm::vec3 s_SectionOffsets[static_cast<int>(SectionId::Count)] = 
    {
        glm::vec3(-5.0f, 0.0f, 0.0f), // Section4
        glm::vec3(0.0f, 0.0f, 0.0f),    // Section5
        glm::vec3(5.0f, 0.0f, 0.0f)    // Section6
    };

    static float s_SectionBaseScale[static_cast<int>(SectionId::Count)] = {1.0f,1.0f,1.0f};

    static float s_SectionUserScale[static_cast<int>(SectionId::Count)] = {1.0f,1.0f,1.0f};

    void SetupScene(Registry& registry, Window& window, DemoSceneType sceneType) 
    {
        SetupMeshScene(registry);
        
        EventSystem::Get().SubscribeToEvent(EventType::KeyPress, [&](const EventData& eventData) 
            {
            if (auto keyCode = std::get_if<int>(&eventData))
            {
                if (*keyCode == Keybinds::KEY_R) 
                {
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
       
        for (int i = 0; i < static_cast<int>(SectionId::Count); ++i) 
        {
            s_SectionEntities[i].clear();
        }
    }

    void ResetScene(Registry& registry, Window& window)
    {
        // Wesley: This probably dosen't work
        ClearScene(registry);        
        SetupScene(registry, window, Systems::g_CurrentDemoScene);
    }

    void SetupMeshScene(Registry& registry)
    {
        auto shader = Systems::g_RenderSystem->GetShader();
        
        const std::string baseUNCPath = "../projects/w.qua-project-4/models/unc/";

        auto loadSectionFromTxts = [&](const std::vector<std::string>& txtFiles, const glm::vec3& offset, SectionId secId)
        {
            const float targetExtent = 0.5f; 

            // First pass: load each mesh once, cache handle and compute largest extent
            float maxExtent = 0.0f;
            struct MeshInfo { std::string path; ResourceHandle handle; glm::vec3 centre; glm::vec3 extents; };
            std::vector<MeshInfo> meshes;
            for (const auto& txt : txtFiles)
            {
                std::ifstream fin(baseUNCPath + txt);
                if (!fin.is_open())
                {
                    std::cerr << "Failed to open UNC list file: " << txt << std::endl;
                    continue;
                }
                std::string relPath;
                while (std::getline(fin, relPath))
                {
                    if (relPath.empty()) continue;
                    std::string fullPath = baseUNCPath + relPath;
                    ResourceHandle meshHandle = ResourceSystem::GetInstance().LoadMesh(fullPath);

                    BoundingComponent tmpBC(meshHandle);
                    const Aabb& aabb = tmpBC.GetAABB();
                    float ext = glm::compMax(aabb.GetExtents());
                    maxExtent = std::max(maxExtent, ext);

                    meshes.push_back({fullPath, meshHandle, aabb.GetCenter(), aabb.GetExtents()});
                }
            }

            if (maxExtent <= 0.0f) maxExtent = 1.0f;
            float baseScale = targetExtent / maxExtent;
            s_SectionBaseScale[static_cast<int>(secId)] = baseScale;

            // Second pass: create entities 
            for (const auto& info : meshes)
            {
                ResourceHandle meshHandle = info.handle;
                glm::vec3 centre = info.centre;

                float initialScale = baseScale * s_SectionUserScale[static_cast<int>(secId)];
                glm::vec3 finalScale(initialScale);
                glm::vec3 finalPos = offset - centre * initialScale;

                auto e = registry.Create();
                registry.AddComponent<TransformComponent>(e, TransformComponent(finalPos, glm::vec3(0.0f), finalScale));

                auto meshRenderer = std::make_shared<MeshRenderer>(meshHandle, glm::vec3(0.0f,1.0f,0.0f));
                BoundingComponent bc(meshHandle);
                bc.InitializeRenderables(shader);

                registry.AddComponent<BoundingComponent>(e, bc);
                registry.AddComponent<RenderComponent>(e, RenderComponent(meshRenderer));

                s_SectionEntities[static_cast<int>(secId)].push_back(e);
            }
        };
 
        // Comment this section out to load no powerplant models
        loadSectionFromTxts({"4a.txt", "4b.txt"}, s_SectionOffsets[0], SectionId::Section4);
        loadSectionFromTxts({"5a.txt", "5b.txt", "5c.txt"}, s_SectionOffsets[1], SectionId::Section5);
        loadSectionFromTxts({"6a.txt", "6b.txt"}, s_SectionOffsets[2], SectionId::Section6);
        
        const std::string basePath = "../projects/w.qua-project-4/models/";
        const std::vector<std::pair<std::string, glm::vec3>> modelData = {
            {"bunny.obj", glm::vec3(-6.0f, 0.0f, -3.0f)},
            {"rhino.obj", glm::vec3(-3.0f, 0.0f, -3.0f)},
            {"cup.obj", glm::vec3(0.0f, 0.0f, -3.0f)},
            {"gun.obj", glm::vec3(3.0f, 0.0f, -3.0f)},
            {"cube.obj", glm::vec3(6.0f, 0.0f, -3.0f)},
            {"arm.obj", glm::vec3(-4.0f, 0.0f, 0.0f)},
            {"cat.obj", glm::vec3(0.0f, 0.0f, 0.0f)},
            {"stuffed.obj", glm::vec3(4.0f, 0.0f, 0.0f)}
        };
        
        // Comment this section out to load no normal models
        //for (const auto& [modelFile, position] : modelData) 
        //{
        //    std::string fullPath = basePath + modelFile;
        //    ResourceHandle meshHandle = ResourceSystem::GetInstance().LoadMesh(fullPath);
        //    
        //    if (meshHandle != INVALID_RESOURCE_HANDLE) 
        //    {
        //        BoundingComponent tmpBC(meshHandle);
        //        const Aabb& aabb = tmpBC.GetAABB();
        //        float maxExtent = glm::compMax(aabb.GetExtents());
        //        float scale = (maxExtent > 0.0f) ? (1.0f / maxExtent) : 1.0f;
        //        
        //        glm::vec3 center = aabb.GetCenter();
        //        glm::vec3 finalPos = position - center * scale;
        //        
        //        auto entity = registry.Create();
        //        registry.AddComponent<TransformComponent>(entity, 
        //            TransformComponent(finalPos, glm::vec3(0.0f), glm::vec3(scale)));
        //        
        //        auto meshRenderer = std::make_shared<MeshRenderer>(meshHandle, glm::vec3(0.0f, 1.0f, 0.0f));
        //        BoundingComponent bc(meshHandle);
        //        bc.InitializeRenderables(shader);
        //        
        //        registry.AddComponent<BoundingComponent>(entity, bc);
        //        registry.AddComponent<RenderComponent>(entity, RenderComponent(meshRenderer));
        //        
        //        s_SectionEntities[static_cast<int>(SectionId::Section5)].push_back(entity);
        //    }
        //}
        
    }

    void SetSectionScale(Registry& registry, SectionId section, float scale)
    {
        int idx = static_cast<int>(section);
        if (idx < 0 || idx >= static_cast<int>(SectionId::Count)) return;

        glm::vec3 offset = s_SectionOffsets[idx];
        float baseScale = s_SectionBaseScale[idx];
        s_SectionUserScale[idx] = scale; // store

        for (auto e : s_SectionEntities[idx])
        {
            if (registry.HasComponent<TransformComponent>(e) && registry.HasComponent<BoundingComponent>(e))
            {
                auto &t = registry.GetComponent<TransformComponent>(e);
                auto &bc = registry.GetComponent<BoundingComponent>(e);
                glm::vec3 centre = bc.GetAABB().GetCenter();

                float finalScale = baseScale * scale;
                t.m_Scale = glm::vec3(finalScale);
                t.m_Position = offset - centre * finalScale;
                t.UpdateModelMatrix();
                EventSystem::Get().FireEvent(EventType::TransformChanged, e);
            }
        }
    }

    float GetSectionScale(Registry& registry, SectionId section)
    {
        int idx = static_cast<int>(section);
        if (idx < 0 || idx >= static_cast<int>(SectionId::Count)) return 1.0f;

        return s_SectionUserScale[idx];
    }
} // namespace DemoScene 