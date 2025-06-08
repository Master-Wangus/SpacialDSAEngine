#pragma once
#include "pch.h"

struct Aabb 
{
    /**
     * @brief Default constructor for AABB with zero bounds.
     */
    Aabb() : min(0.0f), max(0.0f) {} 
    
    /**
     * @brief Constructs an AABB with specified min and max points.
     * @param min Minimum point of the bounding box
     * @param max Maximum point of the bounding box
     */
    Aabb(const glm::vec3& min, const glm::vec3& max);
    
    /**
     * @brief Constructs an AABB with center and uniform extents.
     * @param center Center point of the bounding box
     * @param extents Uniform extents from center
     */
    Aabb(const glm::vec3& center, float extents);
    
    /**
     * @brief Gets the center point of the AABB.
     * @return Center point
     */
    glm::vec3 GetCenter() const;
    
    /**
     * @brief Gets the extents (half-sizes) of the AABB.
     * @return Extents vector
     */
    glm::vec3 GetExtents() const;
    
    /**
     * @brief Sets the center of the AABB while maintaining size.
     * @param center New center point
     */
    void SetCenter(const glm::vec3& center);

    /**
     * @brief Transforms the AABB by a transformation matrix.
     * @param transform Transformation matrix to apply
     */
    void Transform(const glm::mat4& transform);

    glm::vec3 min; 
    glm::vec3 max; 
};

struct Sphere 
{
    /**
     * @brief Default constructor for sphere with zero radius at origin.
     */
    Sphere() : center(0.0f), radius(0.0f) {}  
    
    /**
     * @brief Constructs a sphere with specified center and radius.
     * @param center Center point of the sphere
     * @param radius Radius of the sphere
     */
    Sphere(const glm::vec3& center, float radius);

    glm::vec3  center; 
    float radius; 

};

struct Obb
{
    /**
     * @brief Default constructor for OBB with identity orientation.
     */
    Obb() : center(0.0f), halfExtents(0.0f) 
    {
        axes[0] = glm::vec3(1.0f, 0.0f, 0.0f);
        axes[1] = glm::vec3(0.0f, 1.0f, 0.0f);
        axes[2] = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    
    /**
     * @brief Constructs an OBB with specified center, axes, and extents.
     * @param center Center point of the OBB
     * @param axes Array of 3 orientation axes
     * @param halfExtents Half-extents along each axis
     */
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