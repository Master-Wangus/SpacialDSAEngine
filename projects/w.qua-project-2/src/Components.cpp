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
    // Using white color for all renderables so frustum culling colors will show properly
    glm::vec3 neutralColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // Get AABB (this will compute it if not already computed)
    const auto& aabb = GetAABB();
    m_AABBRenderable = std::make_shared<CubeRenderer>(
        aabb.GetCenter(),
        aabb.GetExtents() * 2.0f, 
        neutralColor,
        true // Wireframe mode
    );
    m_AABBRenderable->Initialize(shader);
    
    // Get Ritter sphere (computed lazily)
    const auto& ritterSphere = GetRitterSphere();
    m_RitterRenderable = std::make_shared<SphereRenderer>(
        ritterSphere.center, 
        ritterSphere.radius,
        neutralColor,
        true 
    );
    m_RitterRenderable->Initialize(shader);
    
    // Get Larsson sphere (computed lazily)
    const auto& larssonSphere = GetLarssonSphere();
    m_LarsonRenderable = std::make_shared<SphereRenderer>(
        larssonSphere.center,
        larssonSphere.radius,
        neutralColor,
        true 
    );
    m_LarsonRenderable->Initialize(shader);
    
    // Get PCA sphere (computed lazily)
    const auto& pcaSphere = GetPCASphere();
    m_PCARenderable = std::make_shared<SphereRenderer>(
        pcaSphere.center, 
        pcaSphere.radius,
        neutralColor,
        true 
    );
    m_PCARenderable->Initialize(shader);
    
    // Get OBB (computed lazily)
    const auto& obb = GetOBB();
    m_OBBRenderable = std::make_shared<CubeRenderer>(
        obb.center,
        obb.axes,
        obb.halfExtents,
        neutralColor,
        true // Wireframe mode
    );
    m_OBBRenderable->Initialize(shader);
}

void BoundingComponent::CleanupRenderables()
{
    m_AABBRenderable.reset();
    m_RitterRenderable.reset();
    m_LarsonRenderable.reset();
    m_PCARenderable.reset();
    m_OBBRenderable.reset();
}

void BoundingComponent::ComputeAABB()
{
    if (m_AABBComputed || m_MeshHandle == INVALID_RESOURCE_HANDLE) return;
    
    const auto& meshResource = ResourceSystem::GetInstance().GetMesh(m_MeshHandle);
    if (!meshResource) return;
    
    Vertex min, max;
    CreateAabbBruteForce(meshResource->GetVertexes().data(), meshResource->GetVertexes().size(), &min, &max);
    m_AABB = Aabb(min.m_Position, max.m_Position);
    m_AABBComputed = true;
}

void BoundingComponent::ComputeRitterSphere()
{
    if (m_RitterComputed || m_MeshHandle == INVALID_RESOURCE_HANDLE) return;
    
    const auto& meshResource = ResourceSystem::GetInstance().GetMesh(m_MeshHandle);
    if (!meshResource) return;
    
    Vertex center;
    float radius;
    CreateSphereRitters(meshResource->GetVertexes().data(), meshResource->GetVertexes().size(), &center, &radius);
    m_RitterSphere = Sphere(center.m_Position, radius);
    m_RitterComputed = true;
}

void BoundingComponent::ComputeLarssonSphere()
{
    if (m_LarssonComputed || m_MeshHandle == INVALID_RESOURCE_HANDLE) return;
    
    const auto& meshResource = ResourceSystem::GetInstance().GetMesh(m_MeshHandle);
    if (!meshResource) return;
    
    Vertex center;
    float radius;
    CreateSphereIterative(meshResource->GetVertexes().data(), meshResource->GetVertexes().size(), 2, 0.5f, &center, &radius);
    m_LarssonSphere = Sphere(center.m_Position, radius);
    m_LarssonComputed = true;
}

void BoundingComponent::ComputePCASphere()
{
    if (m_PCAComputed || m_MeshHandle == INVALID_RESOURCE_HANDLE) return;
    
    const auto& meshResource = ResourceSystem::GetInstance().GetMesh(m_MeshHandle);
    if (!meshResource) return;
    
    Vertex center;
    float radius;
    CreateSpherePCA(meshResource->GetVertexes().data(), meshResource->GetVertexes().size(), &center, &radius);
    m_PCASphere = Sphere(center.m_Position, radius);
    m_PCAComputed = true;
}

void BoundingComponent::ComputeOBB()
{
    if (m_OBBComputed || m_MeshHandle == INVALID_RESOURCE_HANDLE) return;
    
    const auto& meshResource = ResourceSystem::GetInstance().GetMesh(m_MeshHandle);
    if (!meshResource) return;
    
    glm::vec3 obbCenter;
    glm::vec3 obbAxes[3];
    glm::vec3 obbHalfExtents;
    CreateObbPCA(meshResource->GetVertexes().data(), meshResource->GetVertexes().size(), 
                 &obbCenter, obbAxes, &obbHalfExtents);
    m_OBB = Obb(obbCenter, obbAxes, obbHalfExtents);
    m_OBBComputed = true;
} 