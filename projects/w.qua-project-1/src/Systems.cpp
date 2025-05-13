#include "../include/Systems.hpp"
#include "../include/Components.hpp"
#include "../include/Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace Systems {
    void updateTransforms(entt::registry& registry) {
        // Update model matrices for all entities with transform components
        auto view = registry.view<TransformComponent>();
        
        for (auto entity : view) {
            auto& transform = view.get<TransformComponent>(entity);
            transform.updateModelMatrix();
        }
    }

    void renderSystem(entt::registry& registry, const Camera& camera) {
        // Get view and projection matrices from camera
        glm::mat4 viewMatrix = camera.getViewMatrix();
        glm::mat4 projMatrix = camera.getProjectionMatrix(
            static_cast<float>(1024) / static_cast<float>(768)); // Hardcoded aspect ratio for now
        
        // First, apply directional light (if exists)
        auto lightView = registry.view<DirectionalLightComponent>();
        DirectionalLight* activeLight = nullptr;
        
        for (auto entity : lightView) {
            auto& lightComponent = lightView.get<DirectionalLightComponent>(entity);
            activeLight = &lightComponent.light;
            break; // Only use first light for now
        }
        
        // Render all entities with transform and mesh components
        auto renderableView = registry.view<TransformComponent, MeshComponent, MaterialComponent>();
        
        for (auto entity : renderableView) {
            auto [transform, mesh, material] = renderableView.get<TransformComponent, MeshComponent, MaterialComponent>(entity);
            
            // Skip if no shader is assigned
            if (!mesh.shader) continue;
            
            // Use shader
            mesh.shader->use();
            
            // Set transformation matrices
            mesh.shader->setMat4("model", transform.model);
            mesh.shader->setMat4("view", viewMatrix);
            mesh.shader->setMat4("projection", projMatrix);
            
            // Set material properties
            mesh.shader->setVec3("material.ambient", material.material.ambient);
            mesh.shader->setVec3("material.diffuse", material.material.diffuse);
            mesh.shader->setVec3("material.specular", material.material.specular);
            mesh.shader->setFloat("material.shininess", material.material.shininess);
            
            // Set light properties (if available)
            if (activeLight) {
                mesh.shader->setVec3("dirLight.direction", activeLight->direction);
                mesh.shader->setVec3("dirLight.ambient", activeLight->ambient);
                mesh.shader->setVec3("dirLight.diffuse", activeLight->diffuse);
                mesh.shader->setVec3("dirLight.specular", activeLight->specular);
            }
            
            // Set view position for specular calculations
            mesh.shader->setVec3("viewPos", camera.getPosition());
            
            // Bind and draw
            mesh.buffer.bind();
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh.buffer.getVertexCount()));
            mesh.buffer.unbind();
        }
    }
    
    void updateColliders(entt::registry& registry) {
        // Update AABB colliders based on transforms
        auto aabbView = registry.view<TransformComponent, AABBComponent, CubeTag>();
        
        for (auto entity : aabbView) {
            auto [transform, aabb] = aabbView.get<TransformComponent, AABBComponent>(entity);
            
            // Update AABB center to match entity position
            aabb.aabb.mCenter = transform.position;
            
            // Update half-extents based on scale
            aabb.aabb.mHalfExtents = transform.scale * 0.5f;
        }
        
        // Update BoundingSphere colliders based on transforms
        auto sphereView = registry.view<TransformComponent, BoundingSphereComponent>();
        
        for (auto entity : sphereView) {
            auto [transform, sphere] = sphereView.get<TransformComponent, BoundingSphereComponent>(entity);
            
            // Update sphere center to match entity position
            sphere.sphere.center = transform.position;
            
            // For non-uniformly scaled entities, take the maximum axis scale as the radius multiplier
            float maxScale = std::max(std::max(transform.scale.x, transform.scale.y), transform.scale.z);
            
            // For cube entities, the sphere's radius is half the cube's diagonal
            if (registry.all_of<CubeTag>(entity)) {
                // Original radius is half the diagonal of a unit cube (√3/2)
                float originalRadius = 0.866025f; // √3/2
                sphere.sphere.radius = originalRadius * maxScale;
            }
        }
    }
    
    void collisionSystem(entt::registry& registry) {
        // Get views for entities with different collision components
        auto aabbView = registry.view<AABBComponent>();
        auto sphereView = registry.view<BoundingSphereComponent>();
        
        // Test collisions between AABBs
        for (auto e1 = aabbView.begin(); e1 != aabbView.end(); ++e1) {
            auto& aabb1 = aabbView.get<AABBComponent>(*e1);
            
            // Test against other AABBs
            for (auto e2 = std::next(e1); e2 != aabbView.end(); ++e2) {
                auto& aabb2 = aabbView.get<AABBComponent>(*e2);
                
                // Perform collision test
                bool collision = Intersection::aabbVsAABB(aabb1.aabb, aabb2.aabb);
                
                if (collision) {
                    // Handle collision (e.g., change color, apply physics, etc.)
                    // For now, just log to console
                    std::cout << "AABB-AABB collision detected between entities " 
                              << static_cast<uint32_t>(*e1) << " and " 
                              << static_cast<uint32_t>(*e2) << std::endl;
                }
            }
            
            // Test against spheres
            for (auto e2 : sphereView) {
                // Skip if it's the same entity
                if (*e1 == e2) continue;
                
                auto& sphere = sphereView.get<BoundingSphereComponent>(e2);
                
                // Perform collision test
                bool collision = Intersection::aabbVsSphere(aabb1.aabb, sphere.sphere);
                
                if (collision) {
                    // Handle collision
                    std::cout << "AABB-Sphere collision detected between entities " 
                              << static_cast<uint32_t>(*e1) << " and " 
                              << static_cast<uint32_t>(e2) << std::endl;
                }
            }
        }
        
        // Test collisions between spheres
        for (auto e1 = sphereView.begin(); e1 != sphereView.end(); ++e1) {
            auto& sphere1 = sphereView.get<BoundingSphereComponent>(*e1);
            
            // Test against other spheres
            for (auto e2 = std::next(e1); e2 != sphereView.end(); ++e2) {
                auto& sphere2 = sphereView.get<BoundingSphereComponent>(*e2);
                
                // Perform collision test
                bool collision = Intersection::sphereVsSphere(sphere1.sphere, sphere2.sphere);
                
                if (collision) {
                    // Handle collision
                    std::cout << "Sphere-Sphere collision detected between entities " 
                              << static_cast<uint32_t>(*e1) << " and " 
                              << static_cast<uint32_t>(*e2) << std::endl;
                }
            }
        }
    }
    
    // Helper function to create cube vertices
    static std::vector<Vertex> createCubeVertices(const glm::vec3& color) {
        std::vector<Vertex> vertices;
        
        // Generate normalized vertex positions for a cube centered at the origin
        float halfSize = 0.5f;
        
        // Define the 8 corners of the cube
        glm::vec3 corners[8] = {
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
        glm::vec3 normals[6] = {
            glm::vec3( 0.0f,  0.0f, -1.0f), // back
            glm::vec3( 0.0f,  0.0f,  1.0f), // front
            glm::vec3(-1.0f,  0.0f,  0.0f), // left
            glm::vec3( 1.0f,  0.0f,  0.0f), // right
            glm::vec3( 0.0f, -1.0f,  0.0f), // bottom
            glm::vec3( 0.0f,  1.0f,  0.0f)  // top
        };
        
        // Define texture coordinates for each vertex of a face
        glm::vec2 texCoords[4] = {
            glm::vec2(0.0f, 0.0f), // bottom-left
            glm::vec2(1.0f, 0.0f), // bottom-right
            glm::vec2(1.0f, 1.0f), // top-right
            glm::vec2(0.0f, 1.0f)  // top-left
        };
        
        // Helper to add a face to the vertices vector
        auto addFace = [&](int v0, int v1, int v2, int v3, int normalIndex) {
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
}
