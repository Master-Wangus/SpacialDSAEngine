#include "Intersection.hpp"
#include <glm/gtx/norm.hpp>
#include <algorithm>
#include <cmath>

bool Intersection::SphereVsSphere(const BoundingSphere& a, const BoundingSphere& b) 
{
    // Calculate the distance vector between centers
    glm::vec3 d = a.m_Center - b.m_Center;
    
    // Calculate the squared distance using dot product
    float distSquared = glm::dot(d, d);
    
    // Calculate the squared sum of radii
    float sumRadiiSquared = (a.m_Radius + b.m_Radius) * (a.m_Radius + b.m_Radius);
    
    // Spheres intersect if the squared distance is less than or equal to the squared sum of radii
    return distSquared <= sumRadiiSquared;
}

bool Intersection::AabbVsAABB(const AABB& a, const AABB& b) 
{
    // Early rejection test - check for separation on any axis
    if (a.GetMin().x > b.GetMax().x || b.GetMin().x > a.GetMax().x) return false;
    if (a.GetMin().y > b.GetMax().y || b.GetMin().y > a.GetMax().y) return false;
    if (a.GetMin().z > b.GetMax().z || b.GetMin().z > a.GetMax().z) return false;
    
    // If we get here, there's overlap on all axes
    return true;
}

bool Intersection::SphereVsAABB(const BoundingSphere& sphere, const AABB& aabb) 
{
    // Find the closest point on the AABB to the sphere's center
    glm::vec3 closestPoint = glm::clamp(
        sphere.m_Center, 
        aabb.GetMin(), 
        aabb.GetMax()
    );
    
    // Calculate squared distance from sphere center to closest point
    float distSquared = glm::distance2(sphere.m_Center, closestPoint);
    
    // Sphere and AABB intersect if the squared distance is less than or equal to the squared radius
    return distSquared <= sphere.m_Radius * sphere.m_Radius;
}

bool Intersection::PointVsSphere(const glm::vec3& point, const BoundingSphere& sphere) 
{
    return glm::distance2(point, sphere.m_Center) <= sphere.m_Radius * sphere.m_Radius;
}

bool Intersection::PointVsAABB(const glm::vec3& point, const AABB& aabb) 
{
    glm::vec3 min = aabb.GetMin();
    glm::vec3 max = aabb.GetMax();
    
    // Early exit optimization - check each axis and return false immediately if outside bounds
    if (point.x < min.x || point.x > max.x) return false;
    if (point.y < min.y || point.y > max.y) return false;
    if (point.z < min.z || point.z > max.z) return false;
    
    // If we get here, the point is inside the AABB on all axes
    return true;
}

bool Intersection::SphereVsRay(const BoundingSphere& sphere, const Ray& ray, float& t) 
{
    // Compute vector from sphere center to ray origin
    glm::vec3 L = ray.m_Origin - sphere.m_Center;
    
    // Coefficients of the quadratic equation
    float a = glm::dot(ray.m_Direction, ray.m_Direction);  // Should be 1.0 if normalized
    float b = 2.0f * glm::dot(ray.m_Direction, L);
    float c = glm::dot(L, L) - sphere.m_Radius * sphere.m_Radius;
    
    // Compute the discriminant to check for real solutions
    float discriminant = b * b - 4.0f * a * c;
    
    // If the discriminant is negative, the ray misses the sphere
    if (discriminant < 0.0f) 
    {
        return false;
    }
    
    // Compute the two possible t values (solutions to the quadratic)
    float sqrtDiscriminant = std::sqrt(discriminant);
    float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
    float t2 = (-b + sqrtDiscriminant) / (2.0f * a);
    
    // If both t values are negative, the sphere is behind the ray
    if (t1 < 0.0f && t2 < 0.0f) 
    {
        return false;
    }
    
    // Choose the nearest positive t
    if (t1 < 0.0f)
    {
        t = t2;
    }
    else if (t2 < 0.0f)
    {
        t = t1;
    }
    else
    {
        t = std::min(t1, t2);
    }
    
    // Return true to indicate intersection
    return true;
}

bool Intersection::RayVsAABB(const Ray& ray, const AABB& aabb, float& t) 
{
    glm::vec3 min = aabb.GetMin();
    glm::vec3 max = aabb.GetMax();
    
    // Step 1: Compute the inverse of the ray's direction vector for all axes
    // Handle division by zero by using a very large value instead
    glm::vec3 invDir;
    invDir.x = (std::fabs(ray.m_Direction.x) < 1e-8f) ? 1e8f : 1.0f / ray.m_Direction.x;
    invDir.y = (std::fabs(ray.m_Direction.y) < 1e-8f) ? 1e8f : 1.0f / ray.m_Direction.y;
    invDir.z = (std::fabs(ray.m_Direction.z) < 1e-8f) ? 1e8f : 1.0f / ray.m_Direction.z;
    
    // Step 2: Compute intersection points (t-values) for each axis
    glm::vec3 tMinVec = (min - ray.m_Origin) * invDir;
    glm::vec3 tMaxVec = (max - ray.m_Origin) * invDir;
    
    // Step 3: Apply element-wise min and max
    glm::vec3 tMin = glm::vec3(
        std::min(tMinVec.x, tMaxVec.x),
        std::min(tMinVec.y, tMaxVec.y),
        std::min(tMinVec.z, tMaxVec.z)
    );
    
    glm::vec3 tMax = glm::vec3(
        std::max(tMinVec.x, tMaxVec.x),
        std::max(tMinVec.y, tMaxVec.y),
        std::max(tMinVec.z, tMaxVec.z)
    );
    
    // Step 4: Find the maximum of the minimums (entry point) and minimum of the maximums (exit point)
    float tEntry = std::max(std::max(tMin.x, tMin.y), tMin.z);
    float tExit = std::min(std::min(tMax.x, tMax.y), tMax.z);
    
    // Step 5: Perform the final intersection check
    if (tExit >= tEntry && tExit >= 0.0f) {
        t = tEntry > 0.0f ? tEntry : tExit; // Use tEntry if it's positive, otherwise use tExit
        return true;
    }
    
    return false;
}

// Möller–Trumbore algorithm
bool Intersection::RayVsTriangle(const Ray& ray, const Triangle& triangle, float& t) 
{
    const float EPSILON = 1e-8f;

    const glm::vec3& v0 = triangle.m_Vertices[0].m_Data;
    const glm::vec3& v1 = triangle.m_Vertices[1].m_Data;
    const glm::vec3& v2 = triangle.m_Vertices[2].m_Data;
    
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    // h determines whether the ray and edge of triangle is parallel or not
    glm::vec3 h = glm::cross(ray.m_Direction, edge2);
    float det = glm::dot(edge1, h);
    
    if (std::fabs(det) < EPSILON)
        return false;
    
    float invDet = 1.0f / det;
    glm::vec3 s = ray.m_Origin - v0;
    float u = invDet * glm::dot(s, h);
    
    if (u < 0.0f || u > 1.0f)
        return false;
    
    glm::vec3 q = glm::cross(s, edge1);
    float v = invDet * glm::dot(ray.m_Direction, q);
    
    if (v < 0.0f || u + v > 1.0f)
        return false;
    
    t = invDet * glm::dot(edge2, q);
    return t > EPSILON;
}

bool Intersection::RayVsPlane(const Ray& ray, const Plane& plane, float& t) 
{
    // Get the plane normal
    glm::vec3 normal = plane.GetNormal();
    
    // Calculate denominator (dot product of ray direction and plane normal)
    float denom = glm::dot(normal, ray.m_Direction);
    
    // Ray and plane are parallel (or nearly parallel) if denominator is close to 0
    if (std::fabs(denom) < 1e-8f) 
    {
        return false;
    }
    
    // Calculate t for intersection
    t = -(glm::dot(normal, ray.m_Origin) + plane.m_Data.w) / denom;
    
    // Check if intersection is in front of the ray origin
    // Accept both positive and negative denominator to handle both sides of the plane
    return t >= 0.0f;
}

bool Intersection::PointVsPlane(const glm::vec3& point, const Plane& plane) 
{
    // A point lies on a plane if the distance is zero
    // Compute signed distance from point to plane
    float distance = glm::dot(plane.GetNormal(), point) + plane.m_Data.w;
    
    // Consider the point on the plane if the distance is very close to zero
    return std::fabs(distance) < 1e-8f;
}

bool Intersection::PointVsTriangle(const glm::vec3& point, const Triangle& triangle) 
{
    const glm::vec3& V0 = triangle.m_Vertices[0].m_Data;
    const glm::vec3& V1 = triangle.m_Vertices[1].m_Data;
    const glm::vec3& V2 = triangle.m_Vertices[2].m_Data;
    const glm::vec3& P = point;
    
    // Compute edge vectors from vertex V0
    glm::vec3 E0 = V1 - V0;
    glm::vec3 E1 = V2 - V0;
    glm::vec3 VP = P - V0;
    
    // Compute dot products for barycentric coordinate calculation
    float d00 = glm::dot(E0, E0);
    float d01 = glm::dot(E0, E1);
    float d11 = glm::dot(E1, E1);
    float d20 = glm::dot(VP, E0);
    float d21 = glm::dot(VP, E1);
    
    // Compute determinant for normalization
    float denom = d00 * d11 - d01 * d01;
    
    // Check for degenerate triangle
    if (std::fabs(denom) < 1e-8f)
    {
        return false;
    }
    
    // Compute barycentric coordinates
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;
    
    // Return true if point is inside triangle (all barycentric coordinates are non-negative)
    return (u >= 0.0f) && (v >= 0.0f) && (w >= 0.0f);
}

bool Intersection::PlaneVsSphere(const Plane& plane, const BoundingSphere& sphere) 
{
    // Step 1: Compute the signed distance from the sphere's center to the plane
    float w = glm::dot(plane.GetNormal(), sphere.m_Center) + plane.m_Data.w;
    
    // Step 2: Calculate the absolute distance to the plane
    float distance = std::fabs(w);
    
    // Step 3: Check if the distance is less than or equal to the radius
    return distance <= sphere.m_Radius;
}

bool Intersection::PlaneVsAABB(const Plane& plane, const AABB& aabb) 
{
    // Get the plane normal
    glm::vec3 normal = plane.GetNormal();
    
    // Step 1: Find the extremal points of the AABB along the plane normal direction
    glm::vec3 pMin = aabb.m_Center;
    glm::vec3 pMax = aabb.m_Center;
    
    // Compute the extremal points based on the normal direction
    // These are easy to compute from center and half-extents
    if (normal.x > 0.0f) 
    {
        pMin.x -= aabb.m_HalfExtents.x;
        pMax.x += aabb.m_HalfExtents.x;
    } else 
    {
        pMin.x += aabb.m_HalfExtents.x;
        pMax.x -= aabb.m_HalfExtents.x;
    }
    
    if (normal.y > 0.0f) {
        pMin.y -= aabb.m_HalfExtents.y;
        pMax.y += aabb.m_HalfExtents.y;
    } else {
        pMin.y += aabb.m_HalfExtents.y;
        pMax.y -= aabb.m_HalfExtents.y;
    }
    
    if (normal.z > 0.0f) {
        pMin.z -= aabb.m_HalfExtents.z;
        pMax.z += aabb.m_HalfExtents.z;
    } else {
        pMin.z += aabb.m_HalfExtents.z;
        pMax.z -= aabb.m_HalfExtents.z;
    }
    
    // Step 2: Calculate signed distances from the plane to these extremal points
    float dMin = glm::dot(normal, pMin) + plane.m_Data.w;
    float dMax = glm::dot(normal, pMax) + plane.m_Data.w;
    
    // Step 3: If signs are different, AABB intersects the plane
    // If both are positive or both are negative, no intersection
    return dMin * dMax <= 0.0f;
} 