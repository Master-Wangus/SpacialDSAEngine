#pragma once

#include <entt/entt.hpp>
#include "Camera.hpp"

// Forward declarations
class Shader;

/**
 * @namespace Systems
 * @brief Contains all ECS systems
 */
namespace Systems {
    /**
     * @brief Updates transform components (model matrices)
     * @param registry The entity registry
     */
    void updateTransforms(entt::registry& registry);

    /**
     * @brief Renders all entities with transform and mesh components
     * @param registry The entity registry
     * @param camera The camera to use for view and projection matrices
     */
    void renderSystem(entt::registry& registry, const Camera& camera);
    
    /**
     * @brief Updates collision components based on transform components
     * @param registry The entity registry
     */
    void updateColliders(entt::registry& registry);
    
    /**
     * @brief Detects and handles collisions between entities
     * @param registry The entity registry
     */
    void collisionSystem(entt::registry& registry);
    
    /**
     * @brief Creates a cube entity
     * @param registry The entity registry
     * @param position Position of the cube
     * @param size Size of the cube
     * @param color Color of the cube
     * @param shader Shader to use for rendering
     * @return The created entity
     */
    entt::entity createCubeEntity(
        entt::registry& registry,
        const glm::vec3& position,
        float size,
        const glm::vec3& color,
        std::shared_ptr<Shader> shader);
    
    /**
     * @brief Creates a directional light entity
     * @param registry The entity registry
     * @param direction Light direction
     * @param ambient Ambient color
     * @param diffuse Diffuse color
     * @param specular Specular color
     * @return The created entity
     */
    entt::entity createDirectionalLight(
        entt::registry& registry,
        const glm::vec3& direction,
        const glm::vec3& ambient,
        const glm::vec3& diffuse,
        const glm::vec3& specular);
} 