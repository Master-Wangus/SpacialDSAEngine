/**
 * @class Intersection
 * @brief Implements 3D geometric collision detection algorithms.
 *
 * This module provides various intersection tests between different geometric primitives
 * such as spheres, rays, AABBs, planes, triangles, and points for collision detection.
 */

#pragma once

#include "pch.h"
#include "Primitives.hpp"

class Intersection 
{
public:
    // Sphere-related tests
    static bool SphereVsSphere(const BoundingSphere& a, const BoundingSphere& b);
    static bool SphereVsAABB(const BoundingSphere& sphere, const AABB& aabb);
    static bool SphereVsRay(const BoundingSphere& sphere, const Ray& ray, float& t);
    
    // AABB-related tests
    static bool AabbVsAABB(const AABB& a, const AABB& b);
    
    // Point-related tests
    static bool PointVsSphere(const glm::vec3& point, const BoundingSphere& sphere);
    static bool PointVsAABB(const glm::vec3& point, const AABB& aabb);
    static bool PointVsTriangle(const glm::vec3& point, const Triangle& triangle);
    static bool PointVsPlane(const glm::vec3& point, const Plane& plane);
    
    // Ray-related tests
    static bool RayVsAABB(const Ray& ray, const AABB& aabb, float& t);
    static bool RayVsTriangle(const Ray& ray, const Triangle& triangle, float& t);
    static bool RayVsPlane(const Ray& ray, const Plane& plane, float& t);
    
    // Plane-related tests
    static bool PlaneVsAABB(const Plane& plane, const AABB& aabb);
    static bool PlaneVsSphere(const Plane& plane, const BoundingSphere& sphere);
}; 