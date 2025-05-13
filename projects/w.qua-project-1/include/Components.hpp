#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "Primitives.hpp"
#include "Lighting.hpp"
#include "Buffer.hpp"
#include "IRenderable.hpp"

class Shader;

// ==================== Transform Components ====================

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

// ==================== Rendering Components ====================

struct RenderComponent 
{
    std::shared_ptr<IRenderable> renderable;
    
    RenderComponent() = default;
    explicit RenderComponent(std::shared_ptr<IRenderable> r) : renderable(std::move(r)) {}
};


struct MeshComponent 
{
    Buffer buffer;
    std::shared_ptr<Shader> shader;
    
    MeshComponent() = default;
    MeshComponent(const std::vector<Vertex>& vertices, std::shared_ptr<Shader> shdr)
        : shader(std::move(shdr))
    {
        buffer.setup(vertices);
    }
};

struct MaterialComponent 
{
    Material material;
    
    MaterialComponent() = default;
    MaterialComponent(const Material& mat) : material(mat) {}
};

// ==================== Physics Components ====================

struct BoundingSphereComponent 
{
    BoundingSphere sphere;
    
    BoundingSphereComponent() = default;
    BoundingSphereComponent(const glm::vec3& center, float radius) : sphere(center, radius) {}
};

struct AABBComponent 
{
    AABB aabb;
    
    AABBComponent() = default;
    AABBComponent(const glm::vec3& center, const glm::vec3& halfExtents) : aabb(center, halfExtents) {}
};

// ==================== Lighting Components ====================


struct DirectionalLightComponent 
{
    DirectionalLight light;
    
    DirectionalLightComponent() = default;
    DirectionalLightComponent(const DirectionalLight& l) : light(l) {}
};

// ==================== Camera Components ====================

enum class CameraType 
{
    FPS
    // More incoming!
};


struct ProjectionComponent 
{
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    
    ProjectionComponent() = default;
    ProjectionComponent(float fov, float nearPlane, float farPlane)
        : fov(fov), nearPlane(nearPlane), farPlane(farPlane) {}
        
    glm::mat4 getProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }
};


struct FPSCameraComponent 
{
    glm::vec3 cameraPosition;
    glm::vec3 cameraFront;
    glm::vec3 cameraUpDirection;
    float yawAngle;
    float pitchAngle;
    float movementSpeed;
    float movementAcceleration;
    float dragFriction;
    glm::vec3 currentVelocity;
    float mouseSensitivity;

    FPSCameraComponent(
        const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f),
        const glm::vec3& front = glm::vec3(0.0f, 0.0f, -1.0f),
        const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f))
        : cameraPosition(position),
          cameraFront(front),
          cameraUpDirection(up),
          yawAngle(-90.0f),  // Default looking forward
          pitchAngle(0.0f),
          movementSpeed(2.5f),
          movementAcceleration(10.0f),
          dragFriction(5.0f),
          currentVelocity(0.0f),
          mouseSensitivity(0.1f)
    {
        updateVectors();
    }

    void updateVectors() 
    {
        // Calculate front vector from yaw and pitch
        glm::vec3 front;
        front.x = cos(glm::radians(yawAngle)) * cos(glm::radians(pitchAngle));
        front.y = sin(glm::radians(pitchAngle));
        front.z = sin(glm::radians(yawAngle)) * cos(glm::radians(pitchAngle));
        cameraFront = glm::normalize(front);
    }

    glm::mat4 getViewMatrix() const 
    {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUpDirection);
    }
};

struct CameraComponent 
{
    ProjectionComponent projection;
    FPSCameraComponent fps;
    CameraType activeCameraType = CameraType::FPS;
    
    CameraComponent() = default;
    CameraComponent(
        const ProjectionComponent& proj,
        const FPSCameraComponent& fpsCamera,
        CameraType type = CameraType::FPS)
        : projection(proj), fps(fpsCamera), activeCameraType(type) {}
    
    glm::mat4 getViewMatrix() const 
    {
        switch (activeCameraType) 
        {
            case CameraType::FPS:
                return fps.getViewMatrix();
            default:
                return fps.getViewMatrix(); // Default to FPS camera
        }
    }
    
    glm::mat4 getProjectionMatrix(float aspectRatio) const 
    {
        return projection.getProjectionMatrix(aspectRatio);
    }
    
    glm::vec3 getPosition() const 
    {
        switch (activeCameraType) 
        {
            case CameraType::FPS:
                return fps.cameraPosition;
            default:
                return fps.cameraPosition; // Default to FPS camera
        }
    }
};

// ==================== Tag Components ====================

struct CubeTag {};
struct SphereTag {}; 