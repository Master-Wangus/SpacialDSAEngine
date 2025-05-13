#include "../include/Systems.hpp"
#include "../include/Components.hpp"
#include "../include/Shader.hpp"
#include "../include/Intersection.hpp"
#include "../include/Window.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

namespace Systems 
{
    // Uniform Buffer Objects (UBOs) for the directional light and materials
    GLuint g_lightUBO = 0;
    GLuint g_materialUBO = 0;
    bool g_initialized = false;

    void updateTransforms(entt::registry& registry) 
    {
        auto view = registry.view<TransformComponent>();
        
        for (auto entity : view) 
        {
            auto& transform = view.get<TransformComponent>(entity);
            transform.updateModelMatrix();
        }
    }
    
    void renderSystem(entt::registry& registry, const Camera& camera) 
    {
        // Initialize UBOs if not already done
        if (!g_initialized) 
        {
            // Create and set up UBO for directional light
            glGenBuffers(1, &g_lightUBO);
            glBindBuffer(GL_UNIFORM_BUFFER, g_lightUBO);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLight), nullptr, GL_STATIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_lightUBO); // Bind to binding point 0
            
            // Create and set up UBO for material
            glGenBuffers(1, &g_materialUBO);
            glBindBuffer(GL_UNIFORM_BUFFER, g_materialUBO);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), nullptr, GL_STATIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, g_materialUBO); // Bind to binding point 1
            
            g_initialized = true;
        }

        // Get all entities with transform, mesh, and material components
        auto view = registry.view<TransformComponent, MeshComponent, MaterialComponent>();
        
        // Find directional light
        auto lightView = registry.view<DirectionalLightComponent>();
        DirectionalLight light;
        
        if (!lightView.empty()) 
        {
            light = lightView.get<DirectionalLightComponent>(*lightView.begin()).light;
            
            // Upload directional light to UBO (only need to do this once)
            glBindBuffer(GL_UNIFORM_BUFFER, g_lightUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DirectionalLight), &light);
        }
        
        // Process each entity
        for (auto entity : view) 
        {
            auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(entity);
            
            // Skip if no shader is assigned
            if (!mesh.shader) continue;
            
            // Use shader
            mesh.shader->use();
            
            // Set transformation matrices
            float aspectRatio = 1024.0f / 768.0f; // Example aspect ratio
            mesh.shader->setMat4("model", transform.model);
            mesh.shader->setMat4("view", camera.getViewMatrix());
            mesh.shader->setMat4("projection", camera.getProjectionMatrix(aspectRatio));
            
            // Bind uniform blocks to their respective binding points
            GLuint lightBlockIndex = glGetUniformBlockIndex(mesh.shader->getID(), "DirectionalLight");
            GLuint materialBlockIndex = glGetUniformBlockIndex(mesh.shader->getID(), "Material");
            
            if (lightBlockIndex != GL_INVALID_INDEX) 
            {
                glUniformBlockBinding(mesh.shader->getID(), lightBlockIndex, 0);
            }
            
            if (materialBlockIndex != GL_INVALID_INDEX)
             {
                glUniformBlockBinding(mesh.shader->getID(), materialBlockIndex, 1);
            }
            
            // Upload material properties for this specific object
            Material mat;
            mat.ambientColor = material.material.ambient;
            mat.ambientIntensity = 1.0f;
            mat.diffuseColor = material.material.diffuse;
            mat.diffuseIntensity = 1.0f;
            mat.specularColor = material.material.specular;
            mat.specularIntensity = 1.0f;
            mat.shininess = material.material.shininess;
            
            glBindBuffer(GL_UNIFORM_BUFFER, g_materialUBO);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &mat);
            
            // Set camera position for specular lighting
            mesh.shader->setVec3("viewPos", camera.getPosition());
            
            // Bind mesh and draw
            mesh.buffer.bind();
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh.buffer.getVertexCount()));
            mesh.buffer.unbind();
        }
    }
    
    void updateColliders(entt::registry& registry) 
    {
        // Update AABB colliders
        auto aabbView = registry.view<TransformComponent, AABBComponent>();
        
        for (auto entity : aabbView) 
        {
            auto [transform, aabb] = aabbView.get<TransformComponent, AABBComponent>(entity);
            
            // Update AABB center based on entity position
            aabb.aabb.mCenter = transform.position;
            
            // Update half-extents based on entity scale
            aabb.aabb.mHalfExtents = glm::vec3(0.5f) * transform.scale;
        }
        
        // Update sphere colliders
        auto sphereView = registry.view<TransformComponent, BoundingSphereComponent>();
        
        for (auto entity : sphereView) 
        {
            auto [transform, sphere] = sphereView.get<TransformComponent, BoundingSphereComponent>(entity);
            
            // Update sphere center based on entity position
            sphere.sphere.center = transform.position;
            
            // Update sphere radius based on entity scale (use max component for uniform scaling)
            float maxScale = std::max(std::max(transform.scale.x, transform.scale.y), transform.scale.z);
            sphere.sphere.radius = 0.866025f * maxScale; // √3/2 * scale
        }
    }
    
    void collisionSystem(entt::registry& registry) 
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
                bool collision = Intersection::aabbVsAABB(aabb1.aabb, aabb2.aabb);
                
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
                bool collision = Intersection::aabbVsSphere(aabb1.aabb, sphere.sphere);
                
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
                bool collision = Intersection::sphereVsSphere(sphere1.sphere, sphere2.sphere);
                
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
    void cameraSystem(entt::registry& registry, const Window& window, float deltaTime) 
    {
        auto view = registry.view<CameraComponent>();
        if (view.empty()) return;
        
        // Get the camera component (assuming only one camera)
        auto entity = *view.begin();
        auto& cameraComp = view.get<CameraComponent>(entity);
        
        // Only handle input for FPS cameras
        if (cameraComp.activeCameraType != CameraType::FPS) return;
        
        auto& camera = cameraComp.fps;
        
        // Calculate acceleration based on input
        glm::vec3 acceleration(0.0f);
        
        if (window.isKeyPressed(GLFW_KEY_W))
            acceleration += camera.cameraFront;
        if (window.isKeyPressed(GLFW_KEY_S))
            acceleration -= camera.cameraFront;
        if (window.isKeyPressed(GLFW_KEY_A))
            acceleration -= glm::normalize(glm::cross(camera.cameraFront, camera.cameraUpDirection));
        if (window.isKeyPressed(GLFW_KEY_D))
            acceleration += glm::normalize(glm::cross(camera.cameraFront, camera.cameraUpDirection));
        
        // Normalize acceleration if not zero
        if (glm::length(acceleration) > 0.001f)
            acceleration = glm::normalize(acceleration) * camera.movementAcceleration;
        
        // Apply drag force
        glm::vec3 drag = -camera.currentVelocity * camera.dragFriction;
        
        // Update velocity using acceleration and drag
        camera.currentVelocity += (acceleration + drag) * deltaTime;
        
        // Update position using velocity
        camera.cameraPosition += camera.currentVelocity * deltaTime;
    }
    
    void processCameraMouseMovement(entt::registry& registry, float xOffset, float yOffset) 
    {
        auto view = registry.view<CameraComponent>();
        if (view.empty()) return;
        
        // Get the camera component (assuming only one camera)
        auto entity = *view.begin();
        auto& cameraComp = view.get<CameraComponent>(entity);
        
        // Only handle mouse movement for FPS cameras
        if (cameraComp.activeCameraType != CameraType::FPS) return;
        
        auto& camera = cameraComp.fps;
        
        // Apply sensitivity
        xOffset *= camera.mouseSensitivity;
        yOffset *= camera.mouseSensitivity;
        
        // Update angles
        camera.yawAngle += xOffset;
        camera.pitchAngle += yOffset;
        
        // Clamp pitch to avoid flipping
        camera.pitchAngle = std::clamp(camera.pitchAngle, -89.0f, 89.0f);
        
        // Update camera vectors
        camera.updateVectors();
    }
    
    // FPSCameraSystem implementation
    FPSCameraSystem::FPSCameraSystem(entt::registry& registry, Window& window)
        : m_registry(registry)
        , m_window(window)
        , m_lastX(window.getWidth() / 2.0f)
        , m_lastY(window.getHeight() / 2.0f)
        , m_firstMouse(true)
    {
        // Find or create a camera entity
        auto view = registry.view<CameraComponent>();
        if (view.empty()) 
        {
            m_activeCameraEntity = createCamera(registry);
        } else {
            m_activeCameraEntity = *view.begin();
        }
        
        // Set up mouse callback
        window.setCursorPosCallback([this](double xpos, double ypos) 
        {
            this->processMouseMovement(xpos, ypos);
        });
    }
    
    void FPSCameraSystem::update(float deltaTime) 
    {
        if (!m_registry.valid(m_activeCameraEntity)) 
        {
            return;
        }
        
        auto& cameraComp = m_registry.get<CameraComponent>(m_activeCameraEntity);
        auto& camera = cameraComp.fps;
        
        // Calculate acceleration based on input
        glm::vec3 acceleration(0.0f);
        
        if (m_window.isKeyPressed(GLFW_KEY_W))
            acceleration += camera.cameraFront;
        if (m_window.isKeyPressed(GLFW_KEY_S))
            acceleration -= camera.cameraFront;
        if (m_window.isKeyPressed(GLFW_KEY_A))
            acceleration -= glm::normalize(glm::cross(camera.cameraFront, camera.cameraUpDirection));
        if (m_window.isKeyPressed(GLFW_KEY_D))
            acceleration += glm::normalize(glm::cross(camera.cameraFront, camera.cameraUpDirection));
        
        // Normalize acceleration if not zero
        if (glm::length(acceleration) > 0.001f)
            acceleration = glm::normalize(acceleration) * camera.movementAcceleration;
        
        // Apply drag force
        glm::vec3 drag = -camera.currentVelocity * camera.dragFriction;
        
        // Update velocity using acceleration and drag
        camera.currentVelocity += (acceleration + drag) * deltaTime;
        
        // Update position using velocity
        camera.cameraPosition += camera.currentVelocity * deltaTime;
    }
    
    entt::entity FPSCameraSystem::getActiveCameraEntity() const 
    {
        return m_activeCameraEntity;
    }
    
    glm::mat4 FPSCameraSystem::getViewMatrix() const 
    {
        if (!m_registry.valid(m_activeCameraEntity)) 
        {
            return glm::mat4(1.0f);
        }
        
        const auto& cameraComp = m_registry.get<CameraComponent>(m_activeCameraEntity);
        return cameraComp.getViewMatrix();
    }
    
    glm::mat4 FPSCameraSystem::getProjectionMatrix(float aspectRatio) const 
    {
        if (!m_registry.valid(m_activeCameraEntity)) 
        {
            return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        }
        
        const auto& cameraComp = m_registry.get<CameraComponent>(m_activeCameraEntity);
        return cameraComp.getProjectionMatrix(aspectRatio);
    }
    
    glm::vec3 FPSCameraSystem::getPosition() const 
    {
        if (!m_registry.valid(m_activeCameraEntity)) 
        {
            return glm::vec3(0.0f, 0.0f, 5.0f);
        }
        
        const auto& cameraComp = m_registry.get<CameraComponent>(m_activeCameraEntity);
        return cameraComp.getPosition();
    }
    
    void FPSCameraSystem::processMouseMovement(double xpos, double ypos) 
    {
        if (m_firstMouse) {
            m_lastX = static_cast<float>(xpos);
            m_lastY = static_cast<float>(ypos);
            m_firstMouse = false;
            return;
        }
        
        float xOffset = static_cast<float>(xpos) - m_lastX;
        float yOffset = m_lastY - static_cast<float>(ypos); // Reversed since y-coordinates go from bottom to top
        
        m_lastX = static_cast<float>(xpos);
        m_lastY = static_cast<float>(ypos);
        
        processCameraMouseMovement(m_registry, xOffset, yOffset);
    }
    
    // Helper function to create cube vertices
    static std::vector<Vertex> createCubeVertices(const glm::vec3& color) 
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
            vertex.position = corners[v0];
            vertex.color = color;
            vertex.normal = normals[normalIndex];
            vertex.uv = texCoords[0];
            vertices.push_back(vertex);
            
            vertex.position = corners[v1];
            vertex.uv = texCoords[1];
            vertices.push_back(vertex);
            
            vertex.position = corners[v2];
            vertex.uv = texCoords[2];
            vertices.push_back(vertex);
            
            // Second triangle (v0, v2, v3)
            vertex.position = corners[v0];
            vertex.uv = texCoords[0];
            vertices.push_back(vertex);
            
            vertex.position = corners[v2];
            vertex.uv = texCoords[2];
            vertices.push_back(vertex);
            
            vertex.position = corners[v3];
            vertex.uv = texCoords[3];
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
    
    entt::entity createCubeEntity(
        entt::registry& registry,
        const glm::vec3& position,
        float size,
        const glm::vec3& color,
        std::shared_ptr<Shader> shader)
    {
        // Create entity
        auto entity = registry.create();
        
        // Create transform component
        auto& transform = registry.emplace<TransformComponent>(
            entity,
            position,          // position
            glm::vec3(0.0f),   // rotation
            glm::vec3(size)    // scale
        );
        
        // Create mesh component with cube vertices
        auto vertices = createCubeVertices(color);
        registry.emplace<MeshComponent>(entity, vertices, shader);
        
        // Create material component
        registry.emplace<MaterialComponent>(
            entity,
            Material(color, color, glm::vec3(0.5f), 32.0f)
        );
        
        // Create collision components
        registry.emplace<AABBComponent>(entity, position, glm::vec3(size * 0.5f));
        registry.emplace<BoundingSphereComponent>(entity, position, size * 0.866025f); // √3/2 * size
        
        // Add tag
        registry.emplace<CubeTag>(entity);
        
        return entity;
    }
    
    entt::entity createDirectionalLight(
        entt::registry& registry,
        const glm::vec3& direction,
        const glm::vec3& ambient,
        const glm::vec3& diffuse,
        const glm::vec3& specular)
    {
        // Create entity
        auto entity = registry.create();
        
        // Create directional light component
        registry.emplace<DirectionalLightComponent>(
            entity,
            DirectionalLight(direction, ambient, diffuse, specular)
        );
        
        return entity;
    }
    
    entt::entity createCamera(
        entt::registry& registry,
        const glm::vec3& position,
        const glm::vec3& up,
        float fov,
        float nearPlane,
        float farPlane,
        CameraType type)
    {
        // Create entity
        auto entity = registry.create();
        
        // Create FPS camera component
        FPSCameraComponent fpsCamera(position, glm::vec3(0.0f, 0.0f, -1.0f), up);
        
        // Create projection component
        ProjectionComponent projection(fov, nearPlane, farPlane);
        
        // Create camera component with the appropriate type
        registry.emplace<CameraComponent>(entity, projection, fpsCamera, type);
        
        return entity;
    }
}
