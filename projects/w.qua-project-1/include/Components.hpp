#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "Primitives.hpp"
#include "Lighting.hpp"
#include "Buffer.hpp"

class Shader;

// Transform component
struct TransformComponent {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::mat4 model;
    
    TransformComponent(
        const glm::vec3& pos = glm::vec3(0.0f),
        const glm::vec3& rot = glm::vec3(0.0f),
        const glm::vec3& scl = glm::vec3(1.0f))
        : position(pos), rotation(rot), scale(scl), model(1.0f)
    {
        updateModelMatrix();
    }
    
    void updateModelMatrix();
};

// Mesh component
struct MeshComponent {
    Buffer buffer;
    std::shared_ptr<Shader> shader;
    
    MeshComponent() = default;
    MeshComponent(const std::vector<Vertex>& vertices, std::shared_ptr<Shader> shdr)
        : shader(std::move(shdr))
    {
        buffer.setup(vertices);
    }
};

// Material component
struct MaterialComponent {
    Material material;
    
    MaterialComponent() = default;
    MaterialComponent(const Material& mat) : material(mat) {}
};

// Tag components for entity types
struct CubeTag {};
struct SphereTag {};

// Collision components
struct BoundingSphereComponent {
    BoundingSphere sphere;
    
    BoundingSphereComponent() = default;
    BoundingSphereComponent(const glm::vec3& center, float radius) : sphere(center, radius) {}
};

struct AABBComponent {
    AABB aabb;
    
    AABBComponent() = default;
    AABBComponent(const glm::vec3& center, const glm::vec3& halfExtents) : aabb(center, halfExtents) {}
};

// Light component for the directional light
struct DirectionalLightComponent {
    DirectionalLight light;
    
    DirectionalLightComponent() = default;
    DirectionalLightComponent(const DirectionalLight& l) : light(l) {}
}; 