/**
 * @file Components.hpp
 * @brief Component definitions for the entity-component system.
 *
 * This file contains the definitions of all components used in the entity-component system,
 * including transform, rendering, collision, camera, and lighting components.
 */

#pragma once

#include "pch.h"
#include "Lighting.hpp"
#include "Buffer.hpp"
#include "IRenderable.hpp"
#include "Shapes.hpp"
#include "CameraSystem.hpp"

class Shader;

// ==================== Transform Components ====================

struct TransformComponent 
{
    glm::vec3 m_Position;
    glm::vec3 m_Rotation;
    glm::vec3 m_Scale;
    glm::mat4 m_Model;
    
    TransformComponent(
        const glm::vec3& pos = glm::vec3(0.0f),
        const glm::vec3& rot = glm::vec3(0.0f),
        const glm::vec3& scl = glm::vec3(1.0f))
        : m_Position(pos), m_Rotation(rot), m_Scale(scl), m_Model(1.0f)
    {
        UpdateModelMatrix();
    }
    
    void UpdateModelMatrix();
};

// ==================== Rendering Components ====================

struct RenderComponent 
{
    std::shared_ptr<IRenderable> m_Renderable;
    
    RenderComponent() = default;
    explicit RenderComponent(std::shared_ptr<IRenderable> r) : m_Renderable(std::move(r)) {}
};

// ==================== Lighting Components ====================

struct DirectionalLightComponent 
{
    DirectionalLight m_Light;
    
    DirectionalLightComponent() = default;
    DirectionalLightComponent(const DirectionalLight& l) : m_Light(l) {}
};

// ==================== Bounding Volume Components ====================

enum class BoundingVolumeType 
{
    AABB,              
    BSphere_Ritter,    
    BSphere_Larsson,    
    BSphere_PCA         
};

struct BoundingComponent
{
    Aabb m_AABB;
    Sphere m_RitterSphere;      // Ritter's Method
    Sphere m_LarssonSphere;     // Modified Larsson's Method
    Sphere m_PCASphere;         // PCA-based Method

    std::shared_ptr<IRenderable> m_AABBRenderable;
    std::shared_ptr<IRenderable> m_SphereRenderable;

    BoundingComponent() = default;

    BoundingComponent(const Aabb& aabb, const Sphere& ritter, const Sphere& larsson, const Sphere& pca)
        : m_AABB(aabb), m_RitterSphere(ritter), m_LarssonSphere(larsson), m_PCASphere(pca) {}
};

// ==================== Camera Components ====================

struct CameraComponent 
{
    Projection m_Projection;
    FPSCamera m_FPS;
    OrbitalCamera m_Orbital;
    CameraType m_ActiveCameraType = CameraType::FPS;
    
    CameraComponent() = default;
    CameraComponent(
        const Projection& proj,
        const FPSCamera& fpsCamera,
        const OrbitalCamera& orbitalCamera = OrbitalCamera(),
        CameraType type = CameraType::FPS)
        : m_Projection(proj), m_FPS(fpsCamera), m_Orbital(orbitalCamera), m_ActiveCameraType(type) {}
    
    glm::mat4 GetViewMatrix() const 
    {
        switch (m_ActiveCameraType) 
        {
            case CameraType::FPS:
                return m_FPS.GetViewMatrix();
            case CameraType::Orbital:
                return m_Orbital.GetViewMatrix();
            default:
                return m_FPS.GetViewMatrix(); // Default to FPS camera
        }
    }
    
    glm::mat4 GetProjectionMatrix(float aspectRatio) const 
    {
        return m_Projection.GetProjectionMatrix(aspectRatio);
    }
    
    glm::vec3 GetPosition() const 
    {
        switch (m_ActiveCameraType) 
        {
            case CameraType::FPS:
                return m_FPS.m_CameraPosition;
            case CameraType::Orbital:
                return m_Orbital.GetCameraPosition();
            default:
                return m_FPS.m_CameraPosition; // Default to FPS camera
        }
    }
};
