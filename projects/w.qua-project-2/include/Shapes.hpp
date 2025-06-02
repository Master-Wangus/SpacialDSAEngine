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

    void Transform(const glm::mat4& transform);

    glm::vec3 min; 
    glm::vec3 max; 
};

struct Sphere 
{
    Sphere() : center(0.0f), radius(0.0f) {}  
    Sphere(const glm::vec3& center, float radius);

    glm::vec3  center; 
    float radius; 

};