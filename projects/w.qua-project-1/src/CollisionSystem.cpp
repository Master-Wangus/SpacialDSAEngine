/**
 * @class CollisionSystem
 * @brief System for detecting and handling collisions between 3D objects.
 *
 * This system processes collision components and performs intersection tests between
 * different 3D primitives, providing collision response and visualization.
 */

#include "CollisionSystem.hpp"
#include "Primitives.hpp"
#include "Intersection.hpp"

CollisionSystem::CollisionSystem(Registry& registry)
    : m_Registry(registry)
{
    InitializeCollisionTests();
}

void CollisionSystem::InitializeCollisionTests()
{
    // Register all collision test functions in the map
    // Sphere vs Others
    m_CollisionTests[{CollisionShapeType::Sphere, CollisionShapeType::Sphere}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            BoundingSphere sphereA(a.m_Sphere.m_Center, a.m_Sphere.m_Radius);
            BoundingSphere sphereB(b.m_Sphere.m_Center, b.m_Sphere.m_Radius);
            return Intersection::SphereVsSphere(sphereA, sphereB);
        };
    
    m_CollisionTests[{CollisionShapeType::Sphere, CollisionShapeType::AABB}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            BoundingSphere sphere(a.m_Sphere.m_Center, a.m_Sphere.m_Radius);
            AABB aabb(b.m_AABB.m_Center, b.m_AABB.m_HalfExtents);
            return Intersection::SphereVsAABB(sphere, aabb);
        };
    
    m_CollisionTests[{CollisionShapeType::Sphere, CollisionShapeType::Plane}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            BoundingSphere sphere(a.m_Sphere.m_Center, a.m_Sphere.m_Radius);
            Plane plane(b.m_Plane.m_Normal.x, b.m_Plane.m_Normal.y, b.m_Plane.m_Normal.z, b.m_Plane.m_Distance);
            return Intersection::PlaneVsSphere(plane, sphere);
        };
    
    m_CollisionTests[{CollisionShapeType::Sphere, CollisionShapeType::Point}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            BoundingSphere sphere(a.m_Sphere.m_Center, a.m_Sphere.m_Radius);
            return Intersection::PointVsSphere(b.m_Point.m_Position, sphere);
        };
    
    m_CollisionTests[{CollisionShapeType::Sphere, CollisionShapeType::Ray}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            BoundingSphere sphere(a.m_Sphere.m_Center, a.m_Sphere.m_Radius);
            Ray ray(b.m_Ray.m_Origin, b.m_Ray.m_Direction);
            float t = 0.0f;
            return Intersection::SphereVsRay(sphere, ray, t) && t <= b.m_Ray.m_Length;
        };
    
    // AABB vs Others
    m_CollisionTests[{CollisionShapeType::AABB, CollisionShapeType::AABB}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            AABB aabbA(a.m_AABB.m_Center, a.m_AABB.m_HalfExtents);
            AABB aabbB(b.m_AABB.m_Center, b.m_AABB.m_HalfExtents);
            return Intersection::AabbVsAABB(aabbA, aabbB);
        };
    
    m_CollisionTests[{CollisionShapeType::AABB, CollisionShapeType::Plane}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            AABB aabb(a.m_AABB.m_Center, a.m_AABB.m_HalfExtents);
            Plane plane(b.m_Plane.m_Normal.x, b.m_Plane.m_Normal.y, b.m_Plane.m_Normal.z, b.m_Plane.m_Distance);
            return Intersection::PlaneVsAABB(plane, aabb);
        };
    
    m_CollisionTests[{CollisionShapeType::AABB, CollisionShapeType::Point}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            AABB aabb(a.m_AABB.m_Center, a.m_AABB.m_HalfExtents);
            return Intersection::PointVsAABB(b.m_Point.m_Position, aabb);
        };
    
    m_CollisionTests[{CollisionShapeType::AABB, CollisionShapeType::Ray}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            AABB aabb(a.m_AABB.m_Center, a.m_AABB.m_HalfExtents);
            Ray ray(b.m_Ray.m_Origin, b.m_Ray.m_Direction);
            float t = 0.0f;
            return Intersection::RayVsAABB(ray, aabb, t) && t <= b.m_Ray.m_Length;
        };
    
    // Plane vs Others
    m_CollisionTests[{CollisionShapeType::Plane, CollisionShapeType::Ray}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            Plane plane(a.m_Plane.m_Normal.x, a.m_Plane.m_Normal.y, a.m_Plane.m_Normal.z, a.m_Plane.m_Distance);
            Ray ray(b.m_Ray.m_Origin, b.m_Ray.m_Direction);
            float t = 0.0f;
            return Intersection::RayVsPlane(ray, plane, t) && t <= b.m_Ray.m_Length;
        };
    
    m_CollisionTests[{CollisionShapeType::Plane, CollisionShapeType::Point}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            Plane plane(a.m_Plane.m_Normal.x, a.m_Plane.m_Normal.y, a.m_Plane.m_Normal.z, a.m_Plane.m_Distance);
            return Intersection::PointVsPlane(b.m_Point.m_Position, plane);
        };
    
    // Point vs Others
    m_CollisionTests[{CollisionShapeType::Point, CollisionShapeType::Triangle}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            Triangle triangle(
                Point3D(b.m_Triangle.m_Vertices[0]),
                Point3D(b.m_Triangle.m_Vertices[1]),
                Point3D(b.m_Triangle.m_Vertices[2]));
            return Intersection::PointVsTriangle(a.m_Point.m_Position, triangle);
        };
    
    // Ray vs Others
    m_CollisionTests[{CollisionShapeType::Ray, CollisionShapeType::Triangle}] = 
        [](const CollisionComponent& a, const CollisionComponent& b) {
            Ray ray(a.m_Ray.m_Origin, a.m_Ray.m_Direction);
            Triangle triangle(
                Point3D(b.m_Triangle.m_Vertices[0]),
                Point3D(b.m_Triangle.m_Vertices[1]),
                Point3D(b.m_Triangle.m_Vertices[2]));
            float t = 0.0f;
            return Intersection::RayVsTriangle(ray, triangle, t) && t <= a.m_Ray.m_Length;
        };
}

void CollisionSystem::DetectCollisions()
{
    // Clear previous frame's collisions
    m_Collisions.clear();
    
    auto view = m_Registry.View<CollisionComponent>();
    
    // Test all pairs of colliders
    for (auto it1 = view.begin(); it1 != view.end(); ++it1)
    {
        auto entity1 = *it1;
        auto& collider1 = m_Registry.GetComponent<CollisionComponent>(entity1);
        
        // Skip if no valid shape
        if (collider1.m_ShapeType == CollisionShapeType::None)
            continue;
        
        for (auto it2 = std::next(it1); it2 != view.end(); ++it2)
        {
            auto entity2 = *it2;

            // Check if collision is already enqueued for this pair
            if (HasCollision(entity1, entity2))
            {
                continue;
            }
            
            auto& collider2 = m_Registry.GetComponent<CollisionComponent>(entity2);
            
            // Skip if no valid shape
            if (collider2.m_ShapeType == CollisionShapeType::None)
                continue;
            
            // Check for collision
            if (this->CheckCollision(collider1, collider2))
            {
                // Create collision pair
                CollisionPair pair;
                pair.entity1 = entity1;
                pair.entity2 = entity2;
                m_Collisions.push_back(pair);
            }
        }
    }
}

bool CollisionSystem::CheckCollision(const CollisionComponent& a, const CollisionComponent& b)
{
    // Try to find a direct test function for the shape types
    auto directTestKey = std::make_pair(a.m_ShapeType, b.m_ShapeType);
    auto directTestIt = m_CollisionTests.find(directTestKey);
    
    if (directTestIt != m_CollisionTests.end())
    {
        // Use the direct test function
        return directTestIt->second(a, b);
    }
    
    // Try to find a reversed test function for the shape types
    auto reversedTestKey = std::make_pair(b.m_ShapeType, a.m_ShapeType);
    auto reversedTestIt = m_CollisionTests.find(reversedTestKey);
    
    if (reversedTestIt != m_CollisionTests.end())
    {
        // Use the reversed test function with swapped arguments
        return reversedTestIt->second(b, a);
    }
    
    // No test function found for this shape type combination
    return false;
}

bool CollisionSystem::HasCollision(Registry::Entity entity1, Registry::Entity entity2)
{
    for (const auto& pair : m_Collisions)
    {
        if (pair.Contains(entity1, entity2))
        {
            return true;
        }
    }
    return false;
}

void CollisionSystem::UpdateColliders()
{
    // Update all colliders based on entity transforms
    auto view = m_Registry.View<TransformComponent, CollisionComponent>();
    for (auto entity : view)
    {
        auto& transform = m_Registry.GetComponent<TransformComponent>(entity);
        auto& collider = m_Registry.GetComponent<CollisionComponent>(entity);
        
        // Update the collision component based on transform
        collider.UpdateTransform(transform.m_Position, transform.m_Scale);
    }
}

const std::vector<CollisionPair>& CollisionSystem::GetCollisions() const
{
    return m_Collisions;
} 