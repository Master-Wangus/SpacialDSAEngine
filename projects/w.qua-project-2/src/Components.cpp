/**
 * @file Components.cpp
 * @brief Implementation of component functionality for the entity-component system.
 *
 * This file contains the implementation of component-specific functionality
 * for the entity-component system, particularly transform operations.
 */

#include "Components.hpp"
#include "CubeRenderer.hpp"
#include "SphereRenderer.hpp"
#include "Geometry.hpp"

void TransformComponent::UpdateModelMatrix()
{
    m_Model = glm::mat4(1.0f);
    m_Model = glm::translate(m_Model, m_Position);
    m_Model = glm::rotate(m_Model, glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    m_Model = glm::rotate(m_Model, glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    m_Model = glm::rotate(m_Model, glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    m_Model = glm::scale(m_Model, m_Scale);
}

void BoundingComponent::InitializeRenderables(const std::shared_ptr<Shader>& shader)
{
    m_AABBRenderable = std::make_shared<CubeRenderer>(
        m_AABB.GetCenter(),
        m_AABB.GetExtents() * 2.0f, 
        glm::vec3(1.0f, 0.0f, 0.0f), // Red color
        true // Wireframe mode
    );
    m_AABBRenderable->Initialize(shader);
    
    m_RitterRenderable = std::make_shared<SphereRenderer>(
        m_RitterSphere.center, 
        m_RitterSphere.radius,
        glm::vec3(0.0f, 0.0f, 1.0f), // Blue color
        true 
    );
    m_RitterRenderable->Initialize(shader);
    
    m_LarsonRenderable = std::make_shared<SphereRenderer>(
        m_LarssonSphere.center,
        m_LarssonSphere.radius,
        glm::vec3(1.0f, 1.0f, 0.0f), // Yellow color
        true 
    );
    m_LarsonRenderable->Initialize(shader);
    
    m_PCARenderable = std::make_shared<SphereRenderer>(
        m_PCASphere.center, 
        m_PCASphere.radius,
        glm::vec3(1.0f, 0.0f, 1.0f), // Magenta color
        true 
    );
    m_PCARenderable->Initialize(shader);
    
    // Initialize OBB renderable
    m_OBBRenderable = std::make_shared<CubeRenderer>(
        m_OBB.center,
        m_OBB.axes,
        m_OBB.halfExtents,
        glm::vec3(0.0f, 1.0f, 1.0f), // Cyan color
        true // Wireframe mode
    );
    m_OBBRenderable->Initialize(shader);
}

void BoundingComponent::CleanupRenderables()
{
    if (m_AABBRenderable) 
    {
        m_AABBRenderable->CleanUp();
        m_AABBRenderable.reset();
    }
    
    if (m_RitterRenderable) 
    {
        m_RitterRenderable->CleanUp();
        m_RitterRenderable.reset();
    }
    
    if (m_LarsonRenderable) 
    {
        m_LarsonRenderable->CleanUp();
        m_LarsonRenderable.reset();
    }
    
    if (m_PCARenderable) 
    {
        m_PCARenderable->CleanUp();
        m_PCARenderable.reset();
    }
    
    if (m_OBBRenderable)
    {
        m_OBBRenderable->CleanUp();
        m_OBBRenderable.reset();
    }
} 