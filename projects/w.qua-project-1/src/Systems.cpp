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
        
        // Set the material directly on the renderable instead of using MaterialComponent
        Material material;
        material.m_AmbientColor = color;
        material.m_DiffuseColor = color;
        material.m_SpecularColor = color;
        material.m_Shininess = 32.0f;
        cubeRenderable->SetMaterial(material);

        registry.AddComponent<TransformComponent>(entity, TransformComponent(position, glm::vec3(0.0f), glm::vec3(size)));
        registry.AddComponent<RenderComponent>(entity, RenderComponent(cubeRenderable));
        // MaterialComponent no longer needed here

        registry.AddComponent<AABBComponent>(entity, AABBComponent(position, glm::vec3(size * 0.5f)));
        registry.AddComponent<BoundingSphereComponent>(entity, BoundingSphereComponent(position, size * 0.866025f));

        return entity;
    }
}

namespace Systems 
{
    // Uniform Buffer Objects (UBOs) for the directional light and materials
    GLuint g_LightUBO = 0;
    GLuint g_MaterialUBO = 0;
    bool g_Initialized = false;

    void UpdateTransforms(entt::registry& registry) 
    {
        auto view = registry.view<TransformComponent>();
        
        for (auto entity : view) 
        {
            auto& transform = view.get<TransformComponent>(entity);
            transform.UpdateModelMatrix();
        }
    }
    
    void RenderSystem(entt::registry& registry, const Camera& camera) 
    {
        // Initialize UBOs if not already done
        if (!g_Initialized) 
        {
            // Create and set up UBO for directional light
            glGenBuffers(1, &g_LightUBO);
            glBindBuffer(GL_UNIFORM_BUFFER, g_LightUBO);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLight), nullptr, GL_STATIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_LightUBO); // Bind to binding point 0
            
            // Create and set up UBO for material
            glGenBuffers(1, &g_MaterialUBO);
            glBindBuffer(GL_UNIFORM_BUFFER, g_MaterialUBO);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), nullptr, GL_STATIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, g_MaterialUBO); // Bind to binding point 1
            
            g_Initialized = true;
        }

        // Get all entities with transform and render components
        auto view = registry.view<TransformComponent, RenderComponent>();
        
        // Find directional light
        auto lightView = registry.view<DirectionalLightComponent>();
        DirectionalLight light;
        
        if (!lightView.empty()) 
        {
            light = lightView.get<DirectionalLightComponent>(*lightView.begin()).m_Light;
            
            // Upload directional light to UBO (only need to do this once)
            glBindBuffer(GL_UNIFORM_BUFFER, g_LightUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DirectionalLight), &light);
        }
        
        // Get view matrix once for this frame
        glm::mat4 viewMatrix = camera.GetViewMatrix();
        
        // Get camera position for specular lighting calculations
        glm::vec3 cameraPosition = camera.GetPosition();
        
        // Get projection matrix for this frame
        float aspectRatio = 1024.0f / 768.0f; // Example aspect ratio
        glm::mat4 projectionMatrix = camera.GetProjectionMatrix(aspectRatio);
        
        // Process each entity
        for (auto entity : view) 
        {
            auto [transform, renderComp] = view.get<TransformComponent, RenderComponent>(entity);
            
            // Skip if no renderable is assigned
            if (!renderComp.m_Renderable) continue;
            
            // Render the entity
            renderComp.m_Renderable->Render(transform.m_Model, viewMatrix, projectionMatrix);
        }
    }
    
    void UpdateColliders(entt::registry& registry) 
    {
        // Update AABB colliders
        auto aabbView = registry.view<TransformComponent, AABBComponent>();
        
        for (auto entity : aabbView) 
        {
            auto [transform, aabb] = aabbView.get<TransformComponent, AABBComponent>(entity);
            
            // Update AABB center based on entity position
            aabb.m_AABB.m_Center = transform.m_Position;
            
            // Update half-extents based on entity scale
            aabb.m_AABB.m_HalfExtents = glm::vec3(0.5f) * transform.m_Scale;
        }
        
        // Update sphere colliders
        auto sphereView = registry.view<TransformComponent, BoundingSphereComponent>();
        
        for (auto entity : sphereView) 
        {
            auto [transform, sphere] = sphereView.get<TransformComponent, BoundingSphereComponent>(entity);
            
            // Update sphere center based on entity position
            sphere.m_Sphere.m_Center = transform.m_Position;
            
            // Update sphere radius based on entity scale (use max component for uniform scaling)
            float maxScale = std::max(std::max(transform.m_Scale.x, transform.m_Scale.y), transform.m_Scale.z);
            sphere.m_Sphere.m_Radius = 0.866025f * maxScale; // √3/2 * scale
        }
    }
    
    void CollisionSystem(entt::registry& registry) 
    {
        // Get views for all entities with colliders
        auto aabbView = registry.view<AABBComponent>();
        auto sphereView = registry.view<BoundingSphereComponent>();
        
        // Test collisions between AABBs
        for (auto e1 = aabbView.begin(); e1 != aabbView.end(); ++e1) 
        {
            auto& aabb1 = aabbView.get<AABBComponent>(*e1);
            
            // Test against other AABBs
            for (auto e2 = std::next(e1); e2 != aabbView.end(); ++e2) 
            {
                auto& aabb2 = aabbView.get<AABBComponent>(*e2);
                
                // Perform collision test
                bool collision = Intersection::AabbVsAABB(aabb1.m_AABB, aabb2.m_AABB);
                
                if (collision) 
                {
                    // Handle collision 
                    // For now, just log to console
                    std::cout << "AABB-AABB collision detected between entities " 
                              << static_cast<uint32_t>(*e1) << " and " 
                              << static_cast<uint32_t>(*e2) << std::endl;
                }
            }
            
            // Test against spheres
            for (auto e2 : sphereView) 
            {
                // Skip if it's the same entity
                if (*e1 == e2) continue;
                
                auto& sphere = sphereView.get<BoundingSphereComponent>(e2);
                
                // Perform collision test
                bool collision = Intersection::AabbVsSphere(aabb1.m_AABB, sphere.m_Sphere);
                
                if (collision) 
                {
                    // Handle collision
                    std::cout << "AABB-Sphere collision detected between entities " 
                              << static_cast<uint32_t>(*e1) << " and " 
                              << static_cast<uint32_t>(e2) << std::endl;
                }
            }
        }
        
        // Test collisions between spheres
        for (auto e1 = sphereView.begin(); e1 != sphereView.end(); ++e1)
        {
            auto& sphere1 = sphereView.get<BoundingSphereComponent>(*e1);
            
            // Test against other spheres
            for (auto e2 = std::next(e1); e2 != sphereView.end(); ++e2) 
            {
                auto& sphere2 = sphereView.get<BoundingSphereComponent>(*e2);
                
                // Perform collision test
                bool collision = Intersection::SphereVsSphere(sphere1.m_Sphere, sphere2.m_Sphere);
                
                if (collision) 
                {
                    // Handle collision
                    std::cout << "Sphere-Sphere collision detected between entities " 
                              << static_cast<uint32_t>(*e1) << " and " 
                              << static_cast<uint32_t>(*e2) << std::endl;
                }
            }
        }
    }
    
    // New Camera System functions
    void CameraSystem(entt::registry& registry, const Window& window, float deltaTime) 
    {
        auto view = registry.view<CameraComponent>();
        if (view.empty()) return;
        
        // Get the camera component (assuming only one camera)
        auto entity = *view.begin();
        auto& cameraComp = view.get<CameraComponent>(entity);
        
        // Only handle input for FPS cameras
        if (cameraComp.m_ActiveCameraType != CameraType::FPS) return;
        
        auto& camera = cameraComp.m_FPS;
        
        // Calculate acceleration based on input
        glm::vec3 acceleration(0.0f);
        
        if (window.IsKeyPressed(GLFW_KEY_W))
            acceleration += camera.m_CameraFront;
        if (window.IsKeyPressed(GLFW_KEY_S))
            acceleration -= camera.m_CameraFront;
        if (window.IsKeyPressed(GLFW_KEY_A))
            acceleration -= glm::normalize(glm::cross(camera.m_CameraFront, camera.m_CameraUpDirection));
        if (window.IsKeyPressed(GLFW_KEY_D))
            acceleration += glm::normalize(glm::cross(camera.m_CameraFront, camera.m_CameraUpDirection));
        
        // Normalize acceleration if not zero
        if (glm::length(acceleration) > 0.001f)
            acceleration = glm::normalize(acceleration) * camera.m_MovementAcceleration;
        
        // Apply drag force
        glm::vec3 drag = -camera.m_CurrentVelocity * camera.m_DragFriction;
        
        // Update velocity using acceleration and drag
        camera.m_CurrentVelocity += (acceleration + drag) * deltaTime;
        
        // Update position using velocity
        camera.m_CameraPosition += camera.m_CurrentVelocity * deltaTime;
    }
    
    void ProcessCameraMouseMovement(entt::registry& registry, float xOffset, float yOffset) 
    {
        auto view = registry.view<CameraComponent>();
        if (view.empty()) return;
        
        // Get the camera component (assuming only one camera)
        auto entity = *view.begin();
        auto& cameraComp = view.get<CameraComponent>(entity);
        
        // Only handle mouse movement for FPS cameras
        if (cameraComp.m_ActiveCameraType != CameraType::FPS) return;
        
        auto& camera = cameraComp.m_FPS;
        
        // Apply sensitivity
        xOffset *= camera.m_MouseSensitivity;
        yOffset *= camera.m_MouseSensitivity;
        
        // Update angles
        camera.m_YawAngle += xOffset;
        camera.m_PitchAngle += yOffset;
        
        // Clamp pitch to avoid flipping
        camera.m_PitchAngle = std::clamp(camera.m_PitchAngle, -89.0f, 89.0f);
        
        // Update camera vectors
        camera.UpdateVectors();
    }
    
    // FPSCameraSystem implementation
    FPSCameraSystem::FPSCameraSystem(entt::registry& registry, Window& window)
        : Camera(glm::vec3(0.0f, 0.0f, 3.0f)), // Initialize base class
          m_Registry(registry),
          m_Window(window),
          m_ActiveCameraEntity(entt::null)
    {
        // Set up mouse callback
        window.SetCursorPosCallback([this](double xpos, double ypos) 
        {
            ProcessMouseMovement(xpos, ypos);
        });
    }
    
    void FPSCameraSystem::Update(float deltaTime) 
    {
        // Get camera component
        auto& cameraComp = m_Registry.get<CameraComponent>(m_ActiveCameraEntity);
        
        // Update camera position based on input
        float velocity = GetMovementSpeed() * deltaTime;
        
        if (m_Window.IsKeyPressed(GLFW_KEY_W)) 
        {
            SetPosition(GetPosition() + GetForward() * velocity);
        }
        if (m_Window.IsKeyPressed(GLFW_KEY_S)) 
        {
            SetPosition(GetPosition() - GetForward() * velocity);
        }
        if (m_Window.IsKeyPressed(GLFW_KEY_A)) 
        {
            SetPosition(GetPosition() - GetRight() * velocity);
        }
        if (m_Window.IsKeyPressed(GLFW_KEY_D)) 
        {
            SetPosition(GetPosition() + GetRight() * velocity);
        }
        
        // Update camera component
        cameraComp.m_FPS.m_CameraPosition = GetPosition();
        cameraComp.m_FPS.m_CameraFront = GetForward();
        cameraComp.m_FPS.m_CameraUpDirection = GetUp();
    }
    
    glm::mat4 FPSCameraSystem::GetViewMatrix() const 
    {
        return glm::lookAt(GetPosition(), GetPosition() + GetForward(), GetUp());
    }
    
    glm::mat4 FPSCameraSystem::GetProjectionMatrix(float aspectRatio) const 
    {
        return glm::perspective(glm::radians(GetFov()), aspectRatio, 0.1f, 100.0f);
    }
    
    glm::vec3 FPSCameraSystem::GetPosition() const 
    {
        return Camera::GetPosition();
    }
    
    glm::vec3 FPSCameraSystem::GetForward() const 
    {
        return Camera::GetForward();
    }
    
    glm::vec3 FPSCameraSystem::GetRight() const 
    {
        return Camera::GetRight();
    }
    
    glm::vec3 FPSCameraSystem::GetUp() const 
    {
        return Camera::GetUp();
    }
    
    entt::entity FPSCameraSystem::GetActiveCameraEntity() const 
    {
        return m_ActiveCameraEntity;
    }
    
    void FPSCameraSystem::ProcessMouseMovement(double xpos, double ypos) 
    {
        if (m_FirstMouse) 
        {
            m_LastX = static_cast<float>(xpos);
            m_LastY = static_cast<float>(ypos);
            m_FirstMouse = false;
        }
        
        float xOffset = static_cast<float>(xpos) - m_LastX;
        float yOffset = m_LastY - static_cast<float>(ypos); // Reversed since y-coordinates go from bottom to top
        
        m_LastX = static_cast<float>(xpos);
        m_LastY = static_cast<float>(ypos);
        
        xOffset *= GetMouseSensitivity();
        yOffset *= GetMouseSensitivity();
        
        SetYaw(GetYaw() + xOffset);
        SetPitch(GetPitch() + yOffset);
        
        // Constrain pitch
        if (GetPitch() > 89.0f) SetPitch(89.0f);
        if (GetPitch() < -89.0f) SetPitch(-89.0f);
        
        // Update front, right and up vectors
        UpdateCameraVectors();
    }
    
    static std::vector<Vertex> CreateCubeVertices(const glm::vec3& color) 
    {
        std::vector<Vertex> vertices;
        
        // Generate normalized vertex positions for a cube centered at the origin
        float halfSize = 0.5f;
        
        // Define the 8 corners of the cube
        glm::vec3 corners[8] = 
        {
            glm::vec3(-halfSize, -halfSize, -halfSize), // 0: left-bottom-back
            glm::vec3( halfSize, -halfSize, -halfSize), // 1: right-bottom-back
            glm::vec3( halfSize,  halfSize, -halfSize), // 2: right-top-back
            glm::vec3(-halfSize,  halfSize, -halfSize), // 3: left-top-back
            glm::vec3(-halfSize, -halfSize,  halfSize), // 4: left-bottom-front
            glm::vec3( halfSize, -halfSize,  halfSize), // 5: right-bottom-front
            glm::vec3( halfSize,  halfSize,  halfSize), // 6: right-top-front
            glm::vec3(-halfSize,  halfSize,  halfSize)  // 7: left-top-front
        };
        
        // Define the normals for each face
        glm::vec3 normals[6] = 
        {
            glm::vec3( 0.0f,  0.0f, -1.0f), // back
            glm::vec3( 0.0f,  0.0f,  1.0f), // front
            glm::vec3(-1.0f,  0.0f,  0.0f), // left
            glm::vec3( 1.0f,  0.0f,  0.0f), // right
            glm::vec3( 0.0f, -1.0f,  0.0f), // bottom
            glm::vec3( 0.0f,  1.0f,  0.0f)  // top
        };
        
        // Define texture coordinates for each vertex of a face
        glm::vec2 texCoords[4] = 
        {
            glm::vec2(0.0f, 0.0f), // bottom-left
            glm::vec2(1.0f, 0.0f), // bottom-right
            glm::vec2(1.0f, 1.0f), // top-right
            glm::vec2(0.0f, 1.0f)  // top-left
        };
        
        // Helper to add a face to the vertices vector
        auto addFace = [&](int v0, int v1, int v2, int v3, int normalIndex) 
        {
            Vertex vertex;
            
            // First triangle (v0, v1, v2)
            vertex.m_Position = corners[v0];
            vertex.m_Color = color;
            vertex.m_Normal = normals[normalIndex];
            vertex.m_UV = texCoords[0];
            vertices.push_back(vertex);
            
            vertex.m_Position = corners[v1];
            vertex.m_UV = texCoords[1];
            vertices.push_back(vertex);
            
            vertex.m_Position = corners[v2];
            vertex.m_UV = texCoords[2];
            vertices.push_back(vertex);
            
            // Second triangle (v0, v2, v3)
            vertex.m_Position = corners[v0];
            vertex.m_UV = texCoords[0];
            vertices.push_back(vertex);
            
            vertex.m_Position = corners[v2];
            vertex.m_UV = texCoords[2];
            vertices.push_back(vertex);
            
            vertex.m_Position = corners[v3];
            vertex.m_UV = texCoords[3];
            vertices.push_back(vertex);
        };
        
        // Add the 6 faces of the cube
        addFace(0, 1, 2, 3, 0); // back face
        addFace(4, 5, 6, 7, 1); // front face
        addFace(0, 3, 7, 4, 2); // left face
        addFace(1, 5, 6, 2, 3); // right face
        addFace(0, 1, 5, 4, 4); // bottom face
        addFace(3, 2, 6, 7, 5); // top face
        
        return vertices;
    }
    
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
        // Create camera
        SetupCamera(registry, window);
        
        // Create lighting
        SetupLighting(registry);
        
        // Create cubes
        CreateCubes(registry, shader);
    }
    
    void UpdateSystems(Registry& registry, Window& window, float deltaTime) 
    {
        // Update camera based on keyboard input
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
            
            // Calculate camera speed
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
        
        // Get the internal entt::registry
        auto& enttRegistry = registry.GetRegistry();
        
        // Update transforms for all entities with transform components
        UpdateTransforms(enttRegistry);
        
        // Update colliders
        UpdateColliders(registry);
        
        // Check for collisions
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
        // Clean up all renderable objects
        for (auto entity : registry.View<RenderComponent>()) 
        {
            registry.GetComponent<RenderComponent>(entity).m_Renderable->CleanUp();
        }
    }
    
    void SetupCamera(Registry& registry, Window& window)
    {
        // Create the camera entity
        auto cameraEntity = registry.Create();
        
        // Add FPS camera component
        FPSCameraComponent fpsCamera(
            glm::vec3(0.0f, 0.0f, 3.0f),  // position
            glm::vec3(0.0f, 0.0f, -1.0f), // front
            glm::vec3(0.0f, 1.0f, 0.0f)   // up
        );
        
        // Add projection component
        ProjectionComponent projection(
            45.0f,  // fov
            0.1f,   // near plane
            100.0f  // far plane
        );
        
        // Create the camera component
        CameraComponent camera;
        camera.m_FPS = fpsCamera;
        camera.m_Projection = projection;
        camera.m_ActiveCameraType = CameraType::FPS;
        
        // Add component to entity
        registry.AddComponent<CameraComponent>(cameraEntity, camera);

        // Calculate initial values for static variables
        float initialX = window.GetWidth() / 2.0f;
        float initialY = window.GetHeight() / 2.0f;
        
        // Set up mouse callback for camera rotation
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
        // Create directional light entity
        auto lightEntity = registry.Create();
        
        // Create directional light that matches the UBO layout in the shader
        DirectionalLight light;
        
        // Direction (world space) - with a clear downward component
        light.m_Direction = glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f);
        
        // Light color - bright white
        light.m_Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        
        // Add the light component to the entity
        registry.AddComponent<DirectionalLightComponent>(lightEntity, DirectionalLightComponent(light));
        
        // If the UBO for the light doesn't exist yet, create it
        static GLuint lightUBO = 0;
        if (lightUBO == 0) {
            glGenBuffers(1, &lightUBO);
            glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLight), &light, GL_STATIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightUBO); // Bind to binding point 0
        } else {
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
