#include "Shapes.hpp"
#include "Geometry.hpp"

Aabb::Aabb(const glm::vec3& minParam, const glm::vec3& maxParam) : min(minParam), max(maxParam)
{

}

Aabb::Aabb(const glm::vec3& centerParam, float extentsParam) : min(centerParam - extentsParam), max(centerParam + extentsParam)
{

}

Aabb Aabb::Transform(const glm::mat4& transform) const
{
    glm::vec3 transformed_min;
    glm::vec3 transformed_max;
    TransformAabb(min, max, transform, &transformed_min, &transformed_max);
    return Aabb(transformed_min, transformed_max);
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

Sphere::Sphere(const glm::vec3& center, float radius) : center(center), radius(radius) {}