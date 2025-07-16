#include "Shapes.hpp"
#include "Geometry.hpp"

Aabb::Aabb(const glm::vec3& minParam, const glm::vec3& maxParam) : min(minParam), max(maxParam)
{

}

Aabb::Aabb(const glm::vec3& centerParam, float extentsParam) : min(centerParam - extentsParam), max(centerParam + extentsParam)
{

}

void Aabb::Transform(const glm::mat4& transform)
{
    TransformAabb(min, max, transform);
}

glm::vec3 Aabb::GetCenter() const
{
    return (min + max) * 0.5f;
}

glm::vec3 Aabb::GetExtents() const
{
    return (max - min) * 0.5f;
}

void Aabb::SetCenter(const glm::vec3& center)
{
    glm::vec3 extents = GetExtents();
    min = center - extents;
    max = center + extents;
}

bool Aabb::Overlaps(const Aabb& other) const
{
    // Two AABBs overlap if they overlap on all three axes
    return (min.x <= other.max.x && max.x >= other.min.x) &&
           (min.y <= other.max.y && max.y >= other.min.y) &&
           (min.z <= other.max.z && max.z >= other.min.z);
}

Sphere::Sphere(const glm::vec3& center, float radius) : center(center), radius(radius) {}