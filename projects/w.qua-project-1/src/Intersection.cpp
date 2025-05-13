#include "../include/Intersection.hpp"
#include <glm/gtx/norm.hpp>
#include <algorithm>
#include <cmath>

// Sphere vs Sphere intersection test
bool Intersection::sphereVsSphere(const BoundingSphere& a, const BoundingSphere& b) {
    // Calculate the squared distance between centers
    float distSquared = glm::distance2(a.center, b.center);
    
    // Calculate the squared sum of radii
    float sumRadiiSquared = (a.radius + b.radius) * (a.radius + b.radius);
    
    // Spheres intersect if the squared distance is less than or equal to the squared sum of radii
    return distSquared <= sumRadiiSquared;
}

// AABB vs AABB intersection test
bool Intersection::aabbVsAABB(const AABB& a, const AABB& b) {
    // Get min and max corners
    glm::vec3 aMin = a.getMin();
    glm::vec3 aMax = a.getMax();
    glm::vec3 bMin = b.getMin();
    glm::vec3 bMax = b.getMax();
    
    // Check for overlap along all axes
    return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
           (aMin.y <= bMax.y && aMax.y >= bMin.y) &&
           (aMin.z <= bMax.z && aMax.z >= bMin.z);
}

// Sphere vs AABB intersection test
bool Intersection::sphereVsAABB(const BoundingSphere& sphere, const AABB& aabb) {
    // Find the closest point on the AABB to the sphere's center
    glm::vec3 closestPoint = glm::clamp(
        sphere.center, 
        aabb.getMin(), 
        aabb.getMax()
    );
    
    // Calculate squared distance from sphere center to closest point
    float distSquared = glm::distance2(sphere.center, closestPoint);
    
    // Sphere and AABB intersect if the squared distance is less than or equal to the squared radius
    return distSquared <= sphere.radius * sphere.radius;
}

// AABB vs Sphere intersection test (delegates to sphereVsAABB)
bool Intersection::aabbVsSphere(const AABB& aabb, const BoundingSphere& sphere) {
    return sphereVsAABB(sphere, aabb);
}

// Point vs Sphere intersection test
bool Intersection::pointVsSphere(const glm::vec3& point, const BoundingSphere& sphere) {
    return glm::distance2(point, sphere.center) <= sphere.radius * sphere.radius;
}

// Sphere vs Point intersection test (delegates to pointVsSphere)
bool Intersection::sphereVsPoint(const BoundingSphere& sphere, const glm::vec3& point) {
    return pointVsSphere(point, sphere);
}

// Point vs AABB intersection test
bool Intersection::pointVsAABB(const glm::vec3& point, const AABB& aabb) {
    glm::vec3 min = aabb.getMin();
    glm::vec3 max = aabb.getMax();
    
    return (point.x >= min.x && point.x <= max.x) &&
           (point.y >= min.y && point.y <= max.y) &&
           (point.z >= min.z && point.z <= max.z);
}

// AABB vs Point intersection test (delegates to pointVsAABB)
bool Intersection::aabbVsPoint(const AABB& aabb, const glm::vec3& point) {
    return pointVsAABB(point, aabb);
}

// Ray vs Sphere intersection test
bool Intersection::rayVsSphere(const Ray& ray, const BoundingSphere& sphere, float& t) {
    glm::vec3 m = ray.origin - sphere.center;
    float b = glm::dot(m, ray.direction);
    float c = glm::dot(m, m) - sphere.radius * sphere.radius;
    
    // Exit if ray origin is outside sphere and ray is pointing away from sphere
    if (c > 0.0f && b > 0.0f) {
        return false;
    }
    
    // Calculate discriminant
    float discriminant = b * b - c;
    
    // Exit if discriminant is negative (no real roots)
    if (discriminant < 0.0f) {
        return false;
    }
    
    // Calculate closest intersection distance
    t = -b - std::sqrt(discriminant);
    
    // If t is negative, the intersection is behind the ray's origin
    if (t < 0.0f) {
        t = -b + std::sqrt(discriminant);
    }
    
    // If t is still negative, the entire sphere is behind the ray's origin
    return t >= 0.0f;
}

// Sphere vs Ray intersection test (delegates to rayVsSphere)
bool Intersection::sphereVsRay(const BoundingSphere& sphere, const Ray& ray, float& t) {
    return rayVsSphere(ray, sphere, t);
}

// Ray vs AABB intersection test (slab method)
bool Intersection::rayVsAABB(const Ray& ray, const AABB& aabb, float& t) {
    glm::vec3 aabbMin = aabb.getMin();
    glm::vec3 aabbMax = aabb.getMax();
    
    float tMin = -INFINITY;
    float tMax = INFINITY;
    
    for (int i = 0; i < 3; ++i) {
        // Handle division by zero
        if (std::fabs(ray.direction[i]) < 1e-8f) {
            // Ray is parallel to slab, check if origin is within slab
            if (ray.origin[i] < aabbMin[i] || ray.origin[i] > aabbMax[i]) {
                return false;
            }
        } else {
            // Compute intersection t values with near and far slab planes
            float invD = 1.0f / ray.direction[i];
            float t1 = (aabbMin[i] - ray.origin[i]) * invD;
            float t2 = (aabbMax[i] - ray.origin[i]) * invD;
            
            // Swap t1 and t2 if necessary
            if (t1 > t2) {
                std::swap(t1, t2);
            }
            
            // Update tMin and tMax
            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);
            
            // Exit if no overlap
            if (tMin > tMax) {
                return false;
            }
        }
    }
    
    // Intersection found
    t = tMin >= 0 ? tMin : tMax;
    
    // Check if intersection is behind the ray's origin
    return t >= 0.0f;
}

// AABB vs Ray intersection test (delegates to rayVsAABB)
bool Intersection::aabbVsRay(const AABB& aabb, const Ray& ray, float& t) {
    return rayVsAABB(ray, aabb, t);
}

// Ray vs Plane intersection test
bool Intersection::rayVsPlane(const Ray& ray, const Plane& plane, float& t) {
    // Get the plane normal
    glm::vec3 normal = plane.getNormal();
    
    // Calculate denominator (dot product of ray direction and plane normal)
    float denom = glm::dot(normal, ray.direction);
    
    // Ray and plane are parallel (or nearly parallel) if denominator is close to 0
    if (std::fabs(denom) < 1e-8f) {
        return false;
    }
    
    // Calculate t for intersection
    t = -(glm::dot(normal, ray.origin) + plane.mData.w) / denom;
    
    // Check if intersection is in front of the ray origin
    return t >= 0.0f;
}

// Point vs Plane intersection test
bool Intersection::pointVsPlane(const glm::vec3& point, const Plane& plane) {
    // A point lies on a plane if the distance is zero
    // Compute signed distance from point to plane
    float distance = glm::dot(plane.getNormal(), point) + plane.mData.w;
    
    // Consider the point on the plane if the distance is very close to zero
    return std::fabs(distance) < 1e-8f;
}

// Ray vs Triangle intersection test (Möller–Trumbore algorithm)
bool Intersection::rayVsTriangle(const Ray& ray, const Triangle& triangle, float& t) {
    const glm::vec3& v0 = triangle.vertices[0].mData;
    const glm::vec3& v1 = triangle.vertices[1].mData;
    const glm::vec3& v2 = triangle.vertices[2].mData;
    
    // Compute edges
    glm::vec3 e1 = v1 - v0;
    glm::vec3 e2 = v2 - v0;
    
    // Begin calculating determinant
    glm::vec3 p = glm::cross(ray.direction, e2);
    
    // Calculate determinant
    float det = glm::dot(e1, p);
    
    // Check if ray is parallel to the triangle
    if (det > -1e-8f && det < 1e-8f) {
        return false;
    }
    
    float invDet = 1.0f / det;
    
    // Calculate distance from v0 to ray origin
    glm::vec3 s = ray.origin - v0;
    
    // Calculate u parameter
    float u = glm::dot(s, p) * invDet;
    
    // Check bounds
    if (u < 0.0f || u > 1.0f) {
        return false;
    }
    
    // Calculate v parameter
    glm::vec3 q = glm::cross(s, e1);
    float v = glm::dot(ray.direction, q) * invDet;
    
    // Check bounds
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }
    
    // Calculate t value
    t = glm::dot(e2, q) * invDet;
    
    // Check if intersection is behind ray origin
    return t >= 0.0f;
}

// Point vs Triangle intersection test (barycentric coordinates)
bool Intersection::pointVsTriangle(const glm::vec3& point, const Triangle& triangle) {
    const glm::vec3& v0 = triangle.vertices[0].mData;
    const glm::vec3& v1 = triangle.vertices[1].mData;
    const glm::vec3& v2 = triangle.vertices[2].mData;
    
    // Compute vectors
    glm::vec3 v0p = point - v0;
    glm::vec3 v1p = point - v1;
    glm::vec3 v2p = point - v2;
    
    // Compute normal vectors for each sub-triangle
    glm::vec3 n = triangle.computeNormal();
    glm::vec3 n1 = glm::cross(v1 - v0, v0p);
    glm::vec3 n2 = glm::cross(v2 - v1, v1p);
    glm::vec3 n3 = glm::cross(v0 - v2, v2p);
    
    // Check if point is inside the triangle
    return (glm::dot(n, n1) >= 0.0f && 
            glm::dot(n, n2) >= 0.0f && 
            glm::dot(n, n3) >= 0.0f);
}

// Plane vs Sphere intersection test
bool Intersection::planeVsSphere(const Plane& plane, const BoundingSphere& sphere) {
    // Calculate the signed distance from sphere center to plane
    float distance = glm::dot(plane.getNormal(), sphere.center) + plane.mData.w;
    
    // Sphere intersects plane if the absolute distance is less than or equal to the radius
    return std::fabs(distance) <= sphere.radius;
}

// Sphere vs Plane intersection test (delegates to planeVsSphere)
bool Intersection::sphereVsPlane(const BoundingSphere& sphere, const Plane& plane) {
    return planeVsSphere(plane, sphere);
}

// Plane vs AABB intersection test
bool Intersection::planeVsAABB(const Plane& plane, const AABB& aabb) {
    // Calculate the positive extends in the direction of the plane normal
    glm::vec3 extents = aabb.mHalfExtents;
    
    // Project half extents onto the plane normal
    float r = extents.x * std::fabs(plane.getNormal().x) +
              extents.y * std::fabs(plane.getNormal().y) +
              extents.z * std::fabs(plane.getNormal().z);
    
    // Calculate signed distance from box center to plane
    float s = glm::dot(plane.getNormal(), aabb.mCenter) + plane.mData.w;
    
    // Intersection if the center's distance to the plane is within the projected radius
    return std::fabs(s) <= r;
}

// AABB vs Plane intersection test (delegates to planeVsAABB)
bool Intersection::aabbVsPlane(const AABB& aabb, const Plane& plane) {
    return planeVsAABB(plane, aabb);
} 