#include "Systems.hpp"
#include "Components.hpp"
#include "Shader.hpp"
#include "Intersection.hpp"
#include "Window.hpp"
#include "Cube.hpp"
#include "Registry.hpp"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

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

namespace Systems 
{

    // Scene Management Functions
    void InitializeSystems(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader) 
    {
        // Set up the scene entities and components
        SetupScene(registry, window, shader);
        
        // Initialize all renderable objects
        for (auto entity : registry.View<RenderComponent>()) 
        {
            registry.GetComponent<RenderComponent>(entity).m_Renderable->Initialize(shader);
        }
    }
    
    void SetupScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader) 
    {
        SetupCamera(registry, window);
        SetupLighting(registry);
        CreateCubes(registry, shader);
    }
    
    void UpdateSystems(Registry& registry, Window& window, float deltaTime) 
    {
        auto cameraView = registry.View<CameraComponent>();
        if (!cameraView.empty())
        {
            auto cameraEntity = *cameraView.begin();
            auto& camera = registry.GetComponent<CameraComponent>(cameraEntity);
            
            // Get keys state
            bool keyW = window.IsKeyPressed(GLFW_KEY_W);
            bool keyS = window.IsKeyPressed(GLFW_KEY_S);
            bool keyA = window.IsKeyPressed(GLFW_KEY_A);
            bool keyD = window.IsKeyPressed(GLFW_KEY_D);
            
            float cameraSpeed = camera.m_FPS.m_MovementSpeed * deltaTime;
            
            // Update camera position
            if (keyW)
                camera.m_FPS.m_CameraPosition += camera.m_FPS.m_CameraFront * cameraSpeed;
            if (keyS)
                camera.m_FPS.m_CameraPosition -= camera.m_FPS.m_CameraFront * cameraSpeed;
            if (keyA)
                camera.m_FPS.m_CameraPosition -= glm::normalize(glm::cross(camera.m_FPS.m_CameraFront, camera.m_FPS.m_CameraUpDirection)) * cameraSpeed;
            if (keyD)
                camera.m_FPS.m_CameraPosition += glm::normalize(glm::cross(camera.m_FPS.m_CameraFront, camera.m_FPS.m_CameraUpDirection)) * cameraSpeed;
        }

        UpdateTransforms(registry);
        UpdateColliders(registry);
        DetectCollisions(registry);
    }
    
    void RenderSystems(Registry& registry, Window& window) 
    {
        // Find the camera to use for rendering
        auto cameraView = registry.View<CameraComponent>();
        if (cameraView.empty()) return;
        
        auto cameraEntity = *cameraView.begin();
        auto& camera = registry.GetComponent<CameraComponent>(cameraEntity);
        
        // Get camera matrices
        float aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
        glm::mat4 viewMatrix = camera.GetViewMatrix();
        glm::mat4 projectionMatrix = camera.GetProjectionMatrix(aspectRatio);
        
        // Get camera position for specular lighting calculations
        glm::vec3 cameraPosition = camera.GetPosition();
        
        // Get directional light
        auto lightView = registry.View<DirectionalLightComponent>();
        DirectionalLight light;
        if (!lightView.empty())
        {
            light = registry.GetComponent<DirectionalLightComponent>(*lightView.begin()).m_Light;
            
            // Update light UBO
            static GLuint lightUBO = 0;
            if (lightUBO == 0) {
                glGenBuffers(1, &lightUBO);
                glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
                glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLight), nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightUBO); // Bind to binding point 0
            }
            
            glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DirectionalLight), &light);
        }
        
        // Render each entity with transform and render components
        auto renderView = registry.View<TransformComponent, RenderComponent>();
        for (auto entity : renderView)
        {
            auto& transform = registry.GetComponent<TransformComponent>(entity);
            auto& renderComp = registry.GetComponent<RenderComponent>(entity);
            
            renderComp.m_Renderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
        }
    }
    
    void ShutdownSystems(Registry& registry) 
    {
        for (auto entity : registry.View<RenderComponent>()) 
        {
            registry.GetComponent<RenderComponent>(entity).m_Renderable->CleanUp();
        }
    }
    
    void SetupCamera(Registry& registry, Window& window)
    {
        auto cameraEntity = registry.Create();
        
        FPSCameraComponent fpsCamera(
            glm::vec3(0.0f, 0.0f, 3.0f),  // position
            glm::vec3(0.0f, 0.0f, -1.0f), // front
            glm::vec3(0.0f, 1.0f, 0.0f)   // up
        );
        
        ProjectionComponent projection(
            45.0f,  // fov
            0.1f,   // near plane
            100.0f  // far plane
        );
        
        CameraComponent camera;
        camera.m_FPS = fpsCamera;
        camera.m_Projection = projection;
        camera.m_ActiveCameraType = CameraType::FPS;
        
        registry.AddComponent<CameraComponent>(cameraEntity, camera);

        float initialX = window.GetWidth() / 2.0f;
        float initialY = window.GetHeight() / 2.0f;
        
        window.SetCursorPosCallback([&registry, cameraEntity, initialX, initialY](double xpos, double ypos) {
            static bool firstMouse = true;
            static float lastX = initialX;
            static float lastY = initialY;
            
            if (firstMouse) {
                lastX = static_cast<float>(xpos);
                lastY = static_cast<float>(ypos);
                firstMouse = false;
            }
            
            float xOffset = static_cast<float>(xpos) - lastX;
            float yOffset = lastY - static_cast<float>(ypos);
            
            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);
            
            // Update camera
            auto& camera = registry.GetComponent<CameraComponent>(cameraEntity);
            
            const float sensitivity = 0.1f;
            xOffset *= sensitivity;
            yOffset *= sensitivity;
            
            camera.m_FPS.m_YawAngle += xOffset;
            camera.m_FPS.m_PitchAngle += yOffset;
            
            // Constrain pitch
            if (camera.m_FPS.m_PitchAngle > 89.0f) camera.m_FPS.m_PitchAngle = 89.0f;
            if (camera.m_FPS.m_PitchAngle < -89.0f) camera.m_FPS.m_PitchAngle = -89.0f;
            
            camera.m_FPS.UpdateVectors();
        });
    }
    
    void SetupLighting(Registry& registry)
    {
        auto lightEntity = registry.Create();
        DirectionalLight light;
        
        // Direction (world space) - with a clear downward component
        light.m_Direction = glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f);
        
        // Light color - bright white
        light.m_Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        
        // Add the light component to the entity
        registry.AddComponent<DirectionalLightComponent>(lightEntity, DirectionalLightComponent(light));
        
        // If the UBO for the light doesn't exist yet, create it
        static GLuint lightUBO = 0;
        if (lightUBO == 0) 
        {
            glGenBuffers(1, &lightUBO);
            glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLight), &light, GL_STATIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightUBO); // Bind to binding point 0
        } 

        else 
        {
            // Update the data
            glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DirectionalLight), &light);
        }
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
    
    void UpdateColliders(Registry& registry)
    {
        // Update AABB colliders
        auto aabbView = registry.View<TransformComponent, AABBComponent>();
        for (auto entity : aabbView)
        {
            auto& transform = registry.GetComponent<TransformComponent>(entity);
            auto& aabb = registry.GetComponent<AABBComponent>(entity);
            
            // Update AABB center based on entity position
            aabb.m_AABB.m_Center = transform.m_Position;
            
            // Update half-extents based on entity scale
            aabb.m_AABB.m_HalfExtents = glm::vec3(0.5f) * transform.m_Scale;
        }
        
        // Update sphere colliders
        auto sphereView = registry.View<TransformComponent, BoundingSphereComponent>();
        for (auto entity : sphereView)
        {
            auto& transform = registry.GetComponent<TransformComponent>(entity);
            auto& sphere = registry.GetComponent<BoundingSphereComponent>(entity);
            
            // Update sphere center based on entity position
            sphere.m_Sphere.m_Center = transform.m_Position;
            
            // Update sphere radius based on entity scale (use max component for uniform scaling)
            float maxScale = std::max(std::max(transform.m_Scale.x, transform.m_Scale.y), transform.m_Scale.z);
            sphere.m_Sphere.m_Radius = 0.866025f * maxScale; // √3/2 * scale
        }
    }
    
    void DetectCollisions(Registry& registry)
    {
        // Get views for all entities with colliders
        auto aabbView = registry.View<AABBComponent>();
        
        // Test collisions between AABBs
        for (auto it1 = aabbView.begin(); it1 != aabbView.end(); ++it1)
        {
            auto entity1 = *it1;
            auto& aabb1 = registry.GetComponent<AABBComponent>(entity1);
            
            // Test against other AABBs
            for (auto it2 = std::next(it1); it2 != aabbView.end(); ++it2)
            {
                auto entity2 = *it2;
                auto& aabb2 = registry.GetComponent<AABBComponent>(entity2);
                
                // Perform AABB vs AABB collision test
                bool collision = false;
                
                // Check if the boxes intersect on all three axes
                if (!(aabb1.m_AABB.m_Center.x + aabb1.m_AABB.m_HalfExtents.x < aabb2.m_AABB.m_Center.x - aabb2.m_AABB.m_HalfExtents.x ||
                      aabb1.m_AABB.m_Center.x - aabb1.m_AABB.m_HalfExtents.x > aabb2.m_AABB.m_Center.x + aabb2.m_AABB.m_HalfExtents.x ||
                      aabb1.m_AABB.m_Center.y + aabb1.m_AABB.m_HalfExtents.y < aabb2.m_AABB.m_Center.y - aabb2.m_AABB.m_HalfExtents.y ||
                      aabb1.m_AABB.m_Center.y - aabb1.m_AABB.m_HalfExtents.y > aabb2.m_AABB.m_Center.y + aabb2.m_AABB.m_HalfExtents.y ||
                      aabb1.m_AABB.m_Center.z + aabb1.m_AABB.m_HalfExtents.z < aabb2.m_AABB.m_Center.z - aabb2.m_AABB.m_HalfExtents.z ||
                      aabb1.m_AABB.m_Center.z - aabb1.m_AABB.m_HalfExtents.z > aabb2.m_AABB.m_Center.z + aabb2.m_AABB.m_HalfExtents.z))
                {
                    collision = true;
                }
                
                if (collision)
                {
                    // Collision detected
                    std::cout << "AABB-AABB collision detected between entities " 
                              << static_cast<uint32_t>(entity1) << " and " 
                              << static_cast<uint32_t>(entity2) << std::endl;
                }
            }
        }
    }
}
