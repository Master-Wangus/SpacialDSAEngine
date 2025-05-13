#include "../include/Primitives.hpp"
#include <limits>
#include <algorithm>

/**
 * @brief Creates a bounding sphere from a set of points (using a simple approach)
 * @param points Vector of points to be enclosed
 * @return A bounding sphere that encloses all points
 */
BoundingSphere BoundingSphere::fromPoints(const std::vector<glm::vec3>& points) {
    if (points.empty()) {
        return BoundingSphere();
    }
    
    // First compute the center as the average of all points
    glm::vec3 center(0.0f);
    for (const auto& point : points) {
        center += point;
    }
    center /= static_cast<float>(points.size());
    
    // Then find the point farthest from the center
    float maxDistSq = 0.0f;
    for (const auto& point : points) {
        float distSq = glm::distance2(center, point);
        maxDistSq = std::max(maxDistSq, distSq);
    }
    
    // Radius is the square root of the maximum squared distance
    float radius = std::sqrt(maxDistSq);
    
    return BoundingSphere(center, radius);
}

/**
 * @brief Creates an AABB from a set of points
 * @param points Vector of points to be enclosed
 * @return An AABB that encloses all points
 */
AABB AABB::fromPoints(const std::vector<glm::vec3>& points) {
    if (points.empty()) {
        return AABB();
    }
    
    // Initialize min and max with extreme values
    glm::vec3 minCorner(std::numeric_limits<float>::max());
    glm::vec3 maxCorner(std::numeric_limits<float>::lowest());
    
    // Find min and max corners
    for (const auto& point : points) {
        minCorner = glm::min(minCorner, point);
        maxCorner = glm::max(maxCorner, point);
    }
    
    // Create AABB from min and max corners
    return fromMinMax(minCorner, maxCorner);
} 