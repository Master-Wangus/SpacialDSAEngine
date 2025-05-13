#pragma once

#include "Primitives.hpp"

class Intersection 
{
public:
    // Sphere-related tests
    static bool sphereVsSphere(const BoundingSphere& a, const BoundingSphere& b);
    static bool sphereVsAABB(const BoundingSphere& sphere, const AABB& aabb);
    static bool sphereVsPoint(const BoundingSphere& sphere, const glm::vec3& point);
    static bool sphereVsRay(const BoundingSphere& sphere, const Ray& ray, float& t);
    static bool sphereVsPlane(const BoundingSphere& sphere, const Plane& plane);
    
    // AABB-related tests
    static bool aabbVsAABB(const AABB& a, const AABB& b);
    static bool aabbVsSphere(const AABB& aabb, const BoundingSphere& sphere);
    static bool aabbVsPoint(const AABB& aabb, const glm::vec3& point);
    static bool aabbVsRay(const AABB& aabb, const Ray& ray, float& t);
    static bool aabbVsPlane(const AABB& aabb, const Plane& plane);
    
    // Point-related tests
    static bool pointVsSphere(const glm::vec3& point, const BoundingSphere& sphere);
    static bool pointVsAABB(const glm::vec3& point, const AABB& aabb);
    static bool pointVsTriangle(const glm::vec3& point, const Triangle& triangle);
    static bool pointVsPlane(const glm::vec3& point, const Plane& plane);
    
    // Ray-related tests
    static bool rayVsSphere(const Ray& ray, const BoundingSphere& sphere, float& t);
    static bool rayVsAABB(const Ray& ray, const AABB& aabb, float& t);
    static bool rayVsTriangle(const Ray& ray, const Triangle& triangle, float& t);
    static bool rayVsPlane(const Ray& ray, const Plane& plane, float& t);
    
    // Plane-related tests
    static bool planeVsAABB(const Plane& plane, const AABB& aabb);
    static bool planeVsSphere(const Plane& plane, const BoundingSphere& sphere);
}; 