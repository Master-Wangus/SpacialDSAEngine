#include "../include/Primitives.hpp"
#include <limits>
#include <algorithm>

BoundingSphere BoundingSphere::fromPoints(const std::vector<glm::vec3>& points) 
{
    if (points.empty()) 
    {
        return BoundingSphere();
    }
    
    // First compute the center as the average of all points
    glm::vec3 center(0.0f);
    for (const auto& point : points) 
    {
        center += point;
    }
    center /= static_cast<float>(points.size());
    
    // Then find the point farthest from the center
    float maxDistSq = 0.0f;
    for (const auto& point : points) 
    {
        // Calculate squared distance manually
        glm::vec3 diff = point - center;
        float distSq = glm::dot(diff, diff);
        maxDistSq = std::max(maxDistSq, distSq);
    }
    
    // Radius is the square root of the maximum squared distance
    float radius = std::sqrt(maxDistSq);
    
    return BoundingSphere(center, radius);
}

AABB AABB::fromPoints(const std::vector<glm::vec3>& points) {
    if (points.empty()) 
    {
        return AABB();
    }
    
    // Initialize min and max with extreme values
    glm::vec3 minCorner(std::numeric_limits<float>::max());
    glm::vec3 maxCorner(std::numeric_limits<float>::lowest());
    
    // Find min and max corners
    for (const auto& point : points) 
    {
        minCorner = glm::min(minCorner, point);
        maxCorner = glm::max(maxCorner, point);
    }
    
    // Create AABB from min and max corners
    return fromMinMax(minCorner, maxCorner);
} 