#include "Intersection.hpp"
#include <glm/gtx/norm.hpp>
#include <algorithm>
#include <cmath>

// Sphere vs Sphere intersection test
bool Intersection::SphereVsSphere(const BoundingSphere& a, const BoundingSphere& b) 
{
    // Calculate the squared distance between centers
    float distSquared = glm::distance2(a.m_Center, b.m_Center);
    
    // Calculate the squared sum of radii
    float sumRadiiSquared = (a.m_Radius + b.m_Radius) * (a.m_Radius + b.m_Radius);
    
    // Spheres intersect if the squared distance is less than or equal to the squared sum of radii
    return distSquared <= sumRadiiSquared;
}

// AABB vs AABB intersection test
bool Intersection::AabbVsAABB(const AABB& a, const AABB& b) 
{
    // Early rejection test - check for separation on any axis
    if (a.GetMin().x > b.GetMax().x || b.GetMin().x > a.GetMax().x) return false;
    if (a.GetMin().y > b.GetMax().y || b.GetMin().y > a.GetMax().y) return false;
    if (a.GetMin().z > b.GetMax().z || b.GetMin().z > a.GetMax().z) return false;
    
    // If we get here, there's overlap on all axes
    return true;
}

// Sphere vs AABB intersection test
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

// Point vs Sphere intersection test
bool Intersection::PointVsSphere(const glm::vec3& point, const BoundingSphere& sphere) 
{
    return glm::distance2(point, sphere.m_Center) <= sphere.m_Radius * sphere.m_Radius;
}

// Point vs AABB intersection test
bool Intersection::PointVsAABB(const glm::vec3& point, const AABB& aabb) 
{
    glm::vec3 min = aabb.GetMin();
    glm::vec3 max = aabb.GetMax();
    
    return (point.x >= min.x && point.x <= max.x) &&
           (point.y >= min.y && point.y <= max.y) &&
           (point.z >= min.z && point.z <= max.z);
}

// Ray vs Sphere intersection test (renamed from RayVsSphere)
bool Intersection::SphereVsRay(const BoundingSphere& sphere, const Ray& ray, float& t) 
{
    glm::vec3 m = ray.m_Origin - sphere.m_Center;
    float b = glm::dot(m, ray.m_Direction);
    float c = glm::dot(m, m) - sphere.m_Radius * sphere.m_Radius;
    
    // Exit if ray origin is outside sphere and ray is pointing away from sphere
    if (c > 0.0f && b > 0.0f) 
    {
        return false;
    }
    
    // Calculate discriminant
    float discriminant = b * b - c;
    
    // Exit if discriminant is negative (no real roots)
    if (discriminant < 0.0f) 
    {
        return false;
    }
    
    // Calculate closest intersection distance
    t = -b - std::sqrt(discriminant);
    
    // If t is negative, the intersection is behind the ray's origin
    if (t < 0.0f) 
    {
        t = -b + std::sqrt(discriminant);
    }
    
    // If t is still negative, the entire sphere is behind the ray's origin
    return t >= 0.0f;
}

// Ray vs AABB intersection test
bool Intersection::RayVsAABB(const Ray& ray, const AABB& aabb, float& t) 
{
    glm::vec3 min = aabb.GetMin();
    glm::vec3 max = aabb.GetMax();
    
    float tmin = (min.x - ray.m_Origin.x) / ray.m_Direction.x;
    float tmax = (max.x - ray.m_Origin.x) / ray.m_Direction.x;
    
    if (tmin > tmax) std::swap(tmin, tmax);
    
    float tymin = (min.y - ray.m_Origin.y) / ray.m_Direction.y;
    float tymax = (max.y - ray.m_Origin.y) / ray.m_Direction.y;
    
    if (tymin > tymax) std::swap(tymin, tymax);
    
    if ((tmin > tymax) || (tymin > tmax))
                return false;
    
    if (tymin > tmin)
        tmin = tymin;
    
    if (tymax < tmax)
        tmax = tymax;
    
    float tzmin = (min.z - ray.m_Origin.z) / ray.m_Direction.z;
    float tzmax = (max.z - ray.m_Origin.z) / ray.m_Direction.z;
    
    if (tzmin > tzmax) std::swap(tzmin, tzmax);
    
    if ((tmin > tzmax) || (tzmin > tmax))
                return false;
    
    if (tzmin > tmin)
        tmin = tzmin;
    
    if (tzmax < tmax)
        tmax = tzmax;
    
    t = tmin;
    return tmax >= 0.0f;
}

// Ray vs Triangle intersection test (Möller–Trumbore algorithm)
bool Intersection::RayVsTriangle(const Ray& ray, const Triangle& triangle, float& t) 
{
    const glm::vec3& v0 = triangle.m_Vertices[0].m_Data;
    const glm::vec3& v1 = triangle.m_Vertices[1].m_Data;
    const glm::vec3& v2 = triangle.m_Vertices[2].m_Data;
    
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(ray.m_Direction, edge2);
    float a = glm::dot(edge1, h);
    
    if (std::fabs(a) < 1e-8f)
        return false;
    
    float f = 1.0f / a;
    glm::vec3 s = ray.m_Origin - v0;
    float u = f * glm::dot(s, h);
    
    if (u < 0.0f || u > 1.0f)
        return false;
    
    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(ray.m_Direction, q);
    
    if (v < 0.0f || u + v > 1.0f)
        return false;
    
    t = f * glm::dot(edge2, q);
    return t > 1e-8f;
}

// Ray vs Plane intersection test
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

// Point vs Plane intersection test
bool Intersection::PointVsPlane(const glm::vec3& point, const Plane& plane) 
{
    // A point lies on a plane if the distance is zero
    // Compute signed distance from point to plane
    float distance = glm::dot(plane.GetNormal(), point) + plane.m_Data.w;
    
    // Consider the point on the plane if the distance is very close to zero
    return std::fabs(distance) < 1e-8f;
}

// Point vs Triangle intersection test
bool Intersection::PointVsTriangle(const glm::vec3& point, const Triangle& triangle) 
{
    const glm::vec3& v0 = triangle.m_Vertices[0].m_Data;
    const glm::vec3& v1 = triangle.m_Vertices[1].m_Data;
    const glm::vec3& v2 = triangle.m_Vertices[2].m_Data;
    
    // Compute vectors
    glm::vec3 v0p = point - v0;
    glm::vec3 v1p = point - v1;
    glm::vec3 v2p = point - v2;
    
    // Compute normal vectors for each sub-triangle
    glm::vec3 n = triangle.ComputeNormal();
    glm::vec3 n1 = glm::cross(v1 - v0, v0p);
    glm::vec3 n2 = glm::cross(v2 - v1, v1p);
    glm::vec3 n3 = glm::cross(v0 - v2, v2p);
    
    // Check if point is inside the triangle
    return (glm::dot(n, n1) >= 0.0f && 
            glm::dot(n, n2) >= 0.0f && 
            glm::dot(n, n3) >= 0.0f);
}

// Plane vs Sphere intersection test
bool Intersection::PlaneVsSphere(const Plane& plane, const BoundingSphere& sphere) 
{
    // Calculate the signed distance from sphere center to plane
    float distance = glm::dot(plane.GetNormal(), sphere.m_Center) + plane.m_Data.w;
    
    // Sphere intersects plane if the absolute distance is less than or equal to the radius
    return std::fabs(distance) <= sphere.m_Radius;
}

// Plane vs AABB intersection test
bool Intersection::PlaneVsAABB(const Plane& plane, const AABB& aabb) 
{
    // Calculate the positive extends in the direction of the plane normal
    glm::vec3 extents = aabb.m_HalfExtents;
    
    // Project half extents onto the plane normal
    float r = extents.x * std::fabs(plane.GetNormal().x) +
              extents.y * std::fabs(plane.GetNormal().y) +
              extents.z * std::fabs(plane.GetNormal().z);
    
    // Calculate signed distance from box center to plane
    float s = glm::dot(plane.GetNormal(), aabb.m_Center) + plane.m_Data.w;
    
    // Intersection if the center's distance to the plane is within the projected radius
    return std::fabs(s) <= r;
} 