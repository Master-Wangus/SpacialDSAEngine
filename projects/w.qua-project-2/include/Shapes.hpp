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

struct Obb
{
    Obb() : center(0.0f), halfExtents(0.0f) 
    {
        axes[0] = glm::vec3(1.0f, 0.0f, 0.0f);
        axes[1] = glm::vec3(0.0f, 1.0f, 0.0f);
        axes[2] = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    
    Obb(const glm::vec3& center, const glm::vec3 axes[3], const glm::vec3& halfExtents)
        : center(center), halfExtents(halfExtents)
    {
        this->axes[0] = axes[0];
        this->axes[1] = axes[1];
        this->axes[2] = axes[2];
    }

    glm::vec3 center;
    glm::vec3 axes[3];
    glm::vec3 halfExtents;
};