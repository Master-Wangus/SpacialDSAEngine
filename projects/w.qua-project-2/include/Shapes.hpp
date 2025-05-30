#pragma once
#include "pch.h"

struct Aabb 
{
    Aabb() : min(0.0f), max(0.0f) {} 
    Aabb(const glm::vec3& min, const glm::vec3& max);
    Aabb(const glm::vec3& center, float extents);
    glm::vec3 GetCenter() const;
    glm::vec3 GetExtents() const;
    void SetCenter(const glm::vec3& center);

    Aabb Transform(const glm::mat4& transform) const;

    glm::vec3 min; ///< Minimum point of the AABB
    glm::vec3 max; ///< Maximum point of the AABB
};

struct Sphere 
{
    Sphere() : center(0.0f), radius(0.0f) {}  
    Sphere(const glm::vec3& center, float radius);

    glm::vec3  center; ///< Center point of the sphere
    float radius; ///< Radius of the sphere

};