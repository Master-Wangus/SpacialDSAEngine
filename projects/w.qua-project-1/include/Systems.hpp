#pragma once

#include <entt/entt.hpp>
#include "Camera.hpp"
#include "Components.hpp"

// Forward declarations
class Shader;
class Window;


namespace Systems 
{
    void updateTransforms(entt::registry& registry);
    void renderSystem(entt::registry& registry, const Camera& camera);
    void updateColliders(entt::registry& registry);
    void collisionSystem(entt::registry& registry);
    void cameraSystem(
        entt::registry& registry, 
        const Window& window, 
        float deltaTime);
    void processCameraMouseMovement(
        entt::registry& registry,
        float xOffset,
        float yOffset);

    class FPSCameraSystem {
    public:
        FPSCameraSystem(entt::registry& registry, Window& window);
        void update(float deltaTime);
        entt::entity getActiveCameraEntity() const;
        glm::mat4 getViewMatrix() const;
        glm::mat4 getProjectionMatrix(float aspectRatio) const;
        glm::vec3 getPosition() const;
        
    private:
        entt::registry& m_registry;
        Window& m_window;
        entt::entity m_activeCameraEntity;
        
        float m_lastX = 0.0f;
        float m_lastY = 0.0f;
        bool m_firstMouse = true;
        
        void processMouseMovement(double xpos, double ypos);
    };
    
    entt::entity createCubeEntity(
        entt::registry& registry,
        const glm::vec3& position,
        float size,
        const glm::vec3& color,
        std::shared_ptr<Shader> shader);
    
    entt::entity createDirectionalLight(
        entt::registry& registry,
        const glm::vec3& direction,
        const glm::vec3& ambient,
        const glm::vec3& diffuse,
        const glm::vec3& specular);
    
    entt::entity createCamera(
        entt::registry& registry,
        const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f),
        const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
        float fov = 45.0f,
        float nearPlane = 0.1f,
        float farPlane = 100.0f,
        CameraType type = CameraType::FPS);
}