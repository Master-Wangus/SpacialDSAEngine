#pragma once

#include <entt/entt.hpp>
#include "Camera.hpp"
#include "Components.hpp"
#include <memory>

// Forward declarations
class Shader;
class Window;
class Registry;

namespace Systems 
{
    // Scene management functions
    void InitializeSystems(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader);
    void SetupScene(Registry& registry, Window& window, const std::shared_ptr<Shader>& shader);
    void UpdateSystems(Registry& registry, Window& window, float deltaTime);
    void RenderSystems(Registry& registry, Window& window);
    void ShutdownSystems(Registry& registry);

    // Existing system functions
    void UpdateTransforms(entt::registry& registry);
    void RenderSystem(entt::registry& registry, const Camera& camera);
    void UpdateColliders(entt::registry& registry);
    void CollisionSystem(entt::registry& registry);
    void CameraSystem(
        entt::registry& registry, 
        const Window& window, 
        float deltaTime);
    void ProcessCameraMouseMovement(
        entt::registry& registry,
        float xOffset,
        float yOffset);

    // Scene setup helpers
    void SetupCamera(Registry& registry, Window& window);
    void SetupLighting(Registry& registry);
    void CreateCubes(Registry& registry, const std::shared_ptr<Shader>& shader);
    void UpdateColliders(Registry& registry);
    void DetectCollisions(Registry& registry);

    class FPSCameraSystem : public Camera {
    public:
        FPSCameraSystem(entt::registry& registry, Window& window);
        
        // Override Camera methods
        void Update(float deltaTime);
        glm::mat4 GetViewMatrix() const override;
        glm::mat4 GetProjectionMatrix(float aspectRatio) const override;
        glm::vec3 GetPosition() const override;
        glm::vec3 GetForward() const override;
        glm::vec3 GetRight() const override;
        glm::vec3 GetUp() const override;
        
        // Additional FPSCameraSystem methods
        entt::entity GetActiveCameraEntity() const;
        void SetActiveCameraEntity(entt::entity entity) { m_ActiveCameraEntity = entity; }
        
    private:
        entt::registry& m_Registry;
        Window& m_Window;
        entt::entity m_ActiveCameraEntity;
        
        float m_LastX = 0.0f;
        float m_LastY = 0.0f;
        bool m_FirstMouse = true;
        
        void ProcessMouseMovement(double xpos, double ypos);
    };
   
}