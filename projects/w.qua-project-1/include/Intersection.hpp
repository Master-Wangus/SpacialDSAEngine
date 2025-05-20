#pragma once

#include "Primitives.hpp"

class Intersection 
{
public:
    // Sphere-related tests
    static bool SphereVsSphere(const BoundingSphere& a, const BoundingSphere& b);
    static bool SphereVsAABB(const BoundingSphere& sphere, const AABB& aabb);
    static bool SphereVsPoint(const BoundingSphere& sphere, const glm::vec3& point);
    static bool SphereVsRay(const BoundingSphere& sphere, const Ray& ray, float& t);
    static bool SphereVsPlane(const BoundingSphere& sphere, const Plane& plane);
    
    // AABB-related tests
    static bool AabbVsAABB(const AABB& a, const AABB& b);
    static bool AabbVsSphere(const AABB& aabb, const BoundingSphere& sphere);
    static bool AabbVsPoint(const AABB& aabb, const glm::vec3& point);
    static bool AabbVsRay(const AABB& aabb, const Ray& ray, float& t);
    static bool AabbVsPlane(const AABB& aabb, const Plane& plane);
    
    // Point-related tests
    static bool PointVsSphere(const glm::vec3& point, const BoundingSphere& sphere);
    static bool PointVsAABB(const glm::vec3& point, const AABB& aabb);
    static bool PointVsTriangle(const glm::vec3& point, const Triangle& triangle);
    static bool PointVsPlane(const glm::vec3& point, const Plane& plane);
    
    // Ray-related tests
    static bool RayVsSphere(const Ray& ray, const BoundingSphere& sphere, float& t);
    static bool RayVsAABB(const Ray& ray, const AABB& aabb, float& t);
    static bool RayVsTriangle(const Ray& ray, const Triangle& triangle, float& t);
    static bool RayVsPlane(const Ray& ray, const Plane& plane, float& t);
    
    // Plane-related tests
    static bool PlaneVsAABB(const Plane& plane, const AABB& aabb);
    static bool PlaneVsSphere(const Plane& plane, const BoundingSphere& sphere);
}; 