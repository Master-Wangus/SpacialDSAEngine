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
#include "Geometry.hpp"
#include "ResourceSystem.hpp"

class Shader;

// ==================== Transform Components ====================

struct TransformComponent 
{
    glm::vec3 m_Position;
    glm::vec3 m_Rotation;
    glm::vec3 m_Scale;
    glm::mat4 m_Model;
    
    /**
     * @brief Constructs a transform component with position, rotation, and scale.
     * @param pos Initial position vector
     * @param rot Initial rotation vector (in degrees)
     * @param scl Initial scale vector
     */
    TransformComponent(
        const glm::vec3& pos = glm::vec3(0.0f),
        const glm::vec3& rot = glm::vec3(0.0f),
        const glm::vec3& scl = glm::vec3(1.0f))
        : m_Position(pos), m_Rotation(rot), m_Scale(scl), m_Model(1.0f)
    {
        UpdateModelMatrix();
    }
    
    /**
     * @brief Updates the model matrix from position, rotation, and scale.
     */
    void UpdateModelMatrix();
};

// ==================== Rendering Components ====================

struct RenderComponent 
{
    std::shared_ptr<IRenderable> m_Renderable;
    bool m_IsVisible = true;  // Flag to control visibility
    
    RenderComponent() = default;
    
    /**
     * @brief Constructs a render component with a renderable object.
     * @param r Shared pointer to a renderable object
     */
    explicit RenderComponent(std::shared_ptr<IRenderable> r) : m_Renderable(std::move(r)), m_IsVisible(true) {}
};

// ==================== Lighting Components ====================

struct DirectionalLightComponent 
{
    DirectionalLight m_Light;
    
    DirectionalLightComponent() = default;
    
    /**
     * @brief Constructs a directional light component.
     * @param l Directional light parameters
     */
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
    // Bounding volume data
    Aabb m_AABB;
    Sphere m_RitterSphere;      
    Sphere m_LarssonSphere;     
    Sphere m_PCASphere;
    // OBB data
    Obb m_OBB;

    // Wireframe renderables for visualization
    std::shared_ptr<IRenderable> m_AABBRenderable;
    std::shared_ptr<IRenderable> m_RitterRenderable;
    std::shared_ptr<IRenderable> m_LarsonRenderable;
    std::shared_ptr<IRenderable> m_PCARenderable;
    std::shared_ptr<IRenderable> m_OBBRenderable;
    
    // Lazy computation flags
    bool m_AABBComputed = false;
    bool m_RitterComputed = false;
    bool m_LarssonComputed = false;
    bool m_PCAComputed = false;
    bool m_OBBComputed = false;
    
    // Store mesh handle for lazy computation
    ResourceHandle m_MeshHandle = INVALID_RESOURCE_HANDLE;

    BoundingComponent() = default;
    
    /**
     * @brief Constructs a bounding component from a mesh resource with lazy computation.
     * @param resourceHandle Handle to the mesh resource for bounding volume calculation
     */
    BoundingComponent(const ResourceHandle& resourceHandle)
        : m_MeshHandle(resourceHandle)
    {
        // Only compute AABB immediately (it's fast and often needed)
        ComputeAABB();
    }
    
    /**
     * @brief Gets the AABB, computing it if necessary.
     */
    const Aabb& GetAABB() 
    {
        if (!m_AABBComputed) ComputeAABB();
        return m_AABB;
    }
    
    /**
     * @brief Gets the Ritter sphere, computing it if necessary.
     */
    const Sphere& GetRitterSphere() 
    {
        if (!m_RitterComputed) ComputeRitterSphere();
        return m_RitterSphere;
    }
    
    /**
     * @brief Gets the Larsson sphere, computing it if necessary.
     */
    const Sphere& GetLarssonSphere() 
    {
        if (!m_LarssonComputed) ComputeLarssonSphere();
        return m_LarssonSphere;
    }
    
    /**
     * @brief Gets the PCA sphere, computing it if necessary.
     */
    const Sphere& GetPCASphere() 
    {
        if (!m_PCAComputed) ComputePCASphere();
        return m_PCASphere;
    }
    
    /**
     * @brief Gets the OBB, computing it if necessary.
     */
    const Obb& GetOBB() 
    {
        if (!m_OBBComputed) ComputeOBB();
        return m_OBB;
    }
    
    /**
     * @brief Initializes renderable objects for visualizing bounding volumes.
     * @param shader Shader to use for rendering the bounding volumes
     */
    void InitializeRenderables(const std::shared_ptr<Shader>& shader);
    
    /**
     * @brief Cleans up all renderable objects for bounding volume visualization.
     */
    void CleanupRenderables();

private:
    void ComputeAABB();
    void ComputeRitterSphere();
    void ComputeLarssonSphere();
    void ComputePCASphere();
    void ComputeOBB();
};

// ==================== Camera Components ====================

struct CameraComponent 
{
    Projection m_Projection;
    FPSCamera m_FPS;
    OrbitalCamera m_Orbital;
    CameraType m_ActiveCameraType = CameraType::FPS;
    
    CameraComponent() = default;
    
    /**
     * @brief Constructs a camera component with specified parameters.
     * @param proj Projection parameters
     * @param fpsCamera FPS camera configuration
     * @param orbitalCamera Orbital camera configuration
     * @param type Active camera type
     */
    CameraComponent(
        const Projection& proj,
        const FPSCamera& fpsCamera,
        const OrbitalCamera& orbitalCamera = OrbitalCamera(),
        CameraType type = CameraType::FPS)
        : m_Projection(proj), m_FPS(fpsCamera), m_Orbital(orbitalCamera), m_ActiveCameraType(type) {}
    
    /**
     * @brief Gets the view matrix for the currently active camera.
     * @return 4x4 view matrix
     */
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
    
    /**
     * @brief Gets the projection matrix for the given aspect ratio.
     * @param aspectRatio Viewport aspect ratio (width/height)
     * @return 4x4 projection matrix
     */
    glm::mat4 GetProjectionMatrix(float aspectRatio) const 
    {
        return m_Projection.GetProjectionMatrix(aspectRatio);
    }
    
    /**
     * @brief Gets the position of the currently active camera.
     * @return Camera position in world space
     */
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
