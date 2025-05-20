#include "../include/Primitives.hpp"
#include <algorithm>
#include <glm/gtx/norm.hpp>

BoundingSphere BoundingSphere::FromPoints(const std::vector<glm::vec3>& points)
{
    if (points.empty())
        return BoundingSphere();

    // Find the center of the points
    glm::vec3 center(0.0f);
    for (const auto& point : points)
        center += point;
    center /= static_cast<float>(points.size());

    // Find the radius (maximum distance from center to any point)
    float maxRadiusSquared = 0.0f;
    for (const auto& point : points)
    {
        float distSquared = glm::distance2(point, center);
        maxRadiusSquared = std::max(maxRadiusSquared, distSquared);
    }

    return BoundingSphere(center, std::sqrt(maxRadiusSquared));
}

AABB AABB::FromPoints(const std::vector<glm::vec3>& points)
{
    if (points.empty())
        return AABB();

    // Find min and max points
    glm::vec3 minPoint = points[0];
    glm::vec3 maxPoint = points[0];

    for (const auto& point : points)
    {
        minPoint = glm::min(minPoint, point);
        maxPoint = glm::max(maxPoint, point);
    }

    return FromMinMax(minPoint, maxPoint);
}

AABB AABB::FromMinMax(const glm::vec3& minPoint, const glm::vec3& maxPoint)
{
    glm::vec3 center = (minPoint + maxPoint) * 0.5f;
    glm::vec3 halfExtents = (maxPoint - minPoint) * 0.5f;
    return AABB(center, halfExtents);
}

