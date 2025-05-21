#include "CollisionSystem.hpp"
#include <iostream>
#include <algorithm>

CollisionSystem::CollisionSystem(Registry& registry)
    : m_Registry(registry)
{
}

void CollisionSystem::DetectCollisions()
{
    // Clear previous frame's collisions
    m_Collisions.clear();
    

    auto aabbView = m_Registry.View<AABBComponent>();
    auto sphereView = m_Registry.View<BoundingSphereComponent>();
    
    for (auto it1 = aabbView.begin(); it1 != aabbView.end(); ++it1)
    {
        auto entity1 = *it1;
        auto& aabb1 = m_Registry.GetComponent<AABBComponent>(entity1);
        
        for (auto it2 = std::next(it1); it2 != aabbView.end(); ++it2)
        {
            auto entity2 = *it2;
            auto& aabb2 = m_Registry.GetComponent<AABBComponent>(entity2);
            
            if (TestAABBvsAABB(aabb1.m_AABB, aabb2.m_AABB))
            {
                // Collision detected
                m_Collisions.push_back({entity1, entity2});
                
                std::cout << "AABB-AABB collision detected between entities " 
                          << static_cast<uint32_t>(entity1) << " and " 
                          << static_cast<uint32_t>(entity2) << std::endl;
            }
        }
    }
    
    // Test collisions between Spheres
    for (auto it1 = sphereView.begin(); it1 != sphereView.end(); ++it1)
    {
        auto entity1 = *it1;
        auto& sphere1 = m_Registry.GetComponent<BoundingSphereComponent>(entity1);
        
        // Test against other Spheres
        for (auto it2 = std::next(it1); it2 != sphereView.end(); ++it2)
        {
            auto entity2 = *it2;
            auto& sphere2 = m_Registry.GetComponent<BoundingSphereComponent>(entity2);
            
            // Perform Sphere vs Sphere collision test
            if (TestSphereVsSphere(sphere1.m_Sphere, sphere2.m_Sphere))
            {
                // Check if this collision already exists from AABB test
                bool alreadyDetected = false;
                for (const auto& pair : m_Collisions)
                {
                    if ((pair.entity1 == entity1 && pair.entity2 == entity2) ||
                        (pair.entity1 == entity2 && pair.entity2 == entity1))
                    {
                        alreadyDetected = true;
                        break;
                    }
                }
                
                if (!alreadyDetected)
                {
                    m_Collisions.push_back({entity1, entity2});
                    
                    std::cout << "Sphere-Sphere collision detected between entities " 
                              << static_cast<uint32_t>(entity1) << " and " 
                              << static_cast<uint32_t>(entity2) << std::endl;
                }
            }
        }
    }
}

void CollisionSystem::UpdateColliders()
{
    // Update AABB colliders
    auto aabbView = m_Registry.View<TransformComponent, AABBComponent>();
    for (auto entity : aabbView)
    {
        auto& transform = m_Registry.GetComponent<TransformComponent>(entity);
        auto& aabb = m_Registry.GetComponent<AABBComponent>(entity);
        
        // Update AABB center based on entity position
        aabb.m_AABB.m_Center = transform.m_Position;
        
        // Update half-extents based on entity scale
        aabb.m_AABB.m_HalfExtents = glm::vec3(0.5f) * transform.m_Scale;
    }
    
    // Update sphere colliders
    auto sphereView = m_Registry.View<TransformComponent, BoundingSphereComponent>();
    for (auto entity : sphereView)
    {
        auto& transform = m_Registry.GetComponent<TransformComponent>(entity);
        auto& sphere = m_Registry.GetComponent<BoundingSphereComponent>(entity);
        
        // Update sphere center based on entity position
        sphere.m_Sphere.m_Center = transform.m_Position;
        
        // Update sphere radius based on entity scale (use max component for uniform scaling)
        float maxScale = std::max(std::max(transform.m_Scale.x, transform.m_Scale.y), transform.m_Scale.z);
        sphere.m_Sphere.m_Radius = 0.866025f * maxScale; // √3/2 * scale
    }
}

const std::vector<CollisionPair>& CollisionSystem::GetCollisions() const
{
    return m_Collisions;
}

bool CollisionSystem::TestAABBvsAABB(const AABB& a, const AABB& b)
{
    // Check if the boxes intersect on all three axes
    return !(a.m_Center.x + a.m_HalfExtents.x < b.m_Center.x - b.m_HalfExtents.x ||
             a.m_Center.x - a.m_HalfExtents.x > b.m_Center.x + b.m_HalfExtents.x ||
             a.m_Center.y + a.m_HalfExtents.y < b.m_Center.y - b.m_HalfExtents.y ||
             a.m_Center.y - a.m_HalfExtents.y > b.m_Center.y + b.m_HalfExtents.y ||
             a.m_Center.z + a.m_HalfExtents.z < b.m_Center.z - b.m_HalfExtents.z ||
             a.m_Center.z - a.m_HalfExtents.z > b.m_Center.z + b.m_HalfExtents.z);
}

bool CollisionSystem::TestSphereVsSphere(const BoundingSphere& a, const BoundingSphere& b)
{
    // Calculate squared distance between centers
    glm::vec3 difference = a.m_Center - b.m_Center;
    float distanceSquared = glm::dot(difference, difference);
    
    // Calculate sum of radii
    float radiusSum = a.m_Radius + b.m_Radius;
    
    // Check if spheres intersect
    return distanceSquared <= (radiusSum * radiusSum);
} 