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
#include <fstream>

namespace DemoScene 
{
    static std::vector<Registry::Entity> s_SectionEntities[static_cast<int>(SectionId::Count)];

    // Fixed offsets for sections (must match those used in loadSectionFromTxts)
    static const glm::vec3 s_SectionOffsets[static_cast<int>(SectionId::Count)] = {
        glm::vec3(-15.0f, 0.0f, 0.0f), // Section4
        glm::vec3(0.0f, 0.0f, 0.0f),    // Section5
        glm::vec3(15.0f, 0.0f, 0.0f)    // Section6
    };

    // Base normalisation scale computed once per section (fits largest mesh to targetExtent)
    static float s_SectionBaseScale[static_cast<int>(SectionId::Count)] = {1.0f,1.0f,1.0f};

    // User slider scale per section (defaults 1.0)
    static float s_SectionUserScale[static_cast<int>(SectionId::Count)] = {1.0f,1.0f,1.0f};

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
       
        for (int i = 0; i < static_cast<int>(SectionId::Count); ++i) 
        {
            s_SectionEntities[i].clear();
        }
    }

    void ResetScene(Registry& registry, Window& window)
    {
        ClearScene(registry);        
        SetupScene(registry, window, Systems::g_CurrentDemoScene);
    }

    void SetupMeshScene(Registry& registry)
    {
        auto shader = Systems::g_RenderSystem->GetShader();
        
        const std::string baseUNCPath = "../projects/w.qua-project-4/models/unc/";

        auto loadSectionFromTxts = [&](const std::vector<std::string>& txtFiles, const glm::vec3& offset, SectionId secId)
        {
            const float targetExtent = 0.5f; // desired maximum half-size per section

            // First pass: compute largest extent among meshes
            float maxExtent = 0.0f;
            std::vector<std::string> objPaths;
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
                    objPaths.push_back(fullPath);
                    // load mesh temporarily to inspect bounds
                    ResourceHandle meshHandleTmp = ResourceSystem::GetInstance().LoadMesh(fullPath);
                    BoundingComponent tmpBC(meshHandleTmp);
                    float ext = glm::compMax(tmpBC.GetAABB().GetExtents());
                    maxExtent = std::max(maxExtent, ext);
                }
            }

            if (maxExtent <= 0.0f) maxExtent = 1.0f;
            float baseScale = targetExtent / maxExtent;
            s_SectionBaseScale[static_cast<int>(secId)] = baseScale;

            // Second pass: actually create entities with base scale
            for (const auto& fullPath : objPaths)
            {
                ResourceHandle meshHandle = ResourceSystem::GetInstance().LoadMesh(fullPath);

                BoundingComponent tempBC(meshHandle);
                glm::vec3 centre = tempBC.GetAABB().GetCenter();

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
 
        loadSectionFromTxts({"4a.txt", "4b.txt"}, glm::vec3(-15.0f, 0.0f, 0.0f), SectionId::Section4);
        loadSectionFromTxts({"5a.txt", "5b.txt", "5c.txt"}, glm::vec3(0.0f), SectionId::Section5);
        loadSectionFromTxts({"6a.txt", "6b.txt"}, glm::vec3(15.0f, 0.0f, 0.0f), SectionId::Section6);
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