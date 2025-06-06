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
        ResourceHandle meshHandleBunny = ResourceSystem::GetInstance().LoadMesh("../projects/w.qua-project-2/models/bunny.obj");
        
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
        
        // Bunny
        auto meshEntityBunny = registry.Create();
        registry.AddComponent<TransformComponent>(meshEntityBunny,
            TransformComponent(glm::vec3(10.0f, 5.0f, 10.0f),
                glm::vec3(45.0f, 30.0f, 0.0f),  // Rotated
                glm::vec3(1.5f)));              // Scaled up
        
        auto meshRendererBunny = std::make_shared<MeshRenderer>(meshHandleBunny, glm::vec3(1.0f, 0.5f, 0.0f), true); // Orange color
        
        // Create bounding component for bunny
        auto boundingComponentBunny = BoundingComponent(meshHandleBunny);
        boundingComponentBunny.InitializeRenderables(shader);
        registry.AddComponent<BoundingComponent>(meshEntityBunny, boundingComponentBunny);
        
        // Create render component for bunny
        RenderComponent renderCompBunny(meshRendererBunny);
        registry.AddComponent<RenderComponent>(meshEntityBunny, renderCompBunny);
        
        // Simple Cube
        auto cubeEntity = registry.Create();
        registry.AddComponent<TransformComponent>(cubeEntity,
            TransformComponent(glm::vec3(-10.0f, 0.0f, 10.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(1.0f)));  // No scaling
        
        // Create cube renderer
        auto cubeRenderer = std::make_shared<CubeRenderer>(
            glm::vec3(0.0f),          // Center (will be positioned by transform)
            glm::vec3(1.0f),          // Size 
            glm::vec3(0.7f, 0.0f, 0.7f), // Purple color
            true                      // Wireframe
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
            v.m_Color = glm::vec3(0.7f, 0.0f, 0.7f);
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
    }
} 