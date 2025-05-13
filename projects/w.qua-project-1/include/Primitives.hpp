#pragma once

#include <glm/glm.hpp>
#include <vector>


struct Point3D 
{
    glm::vec3 mData;  ///< Position data (x, y, z)

    Point3D() : mData(0.0f) {}
    Point3D(float x, float y, float z) : mData(x, y, z) {}
    explicit Point3D(const glm::vec3& data) : mData(data) {}
};


struct Plane 
{
    glm::vec4 mData;  ///< Plane equation coefficients (nx, ny, nz, d)

    Plane() : mData(0.0f, 0.0f, 0.0f, 0.0f) {}
    Plane(float nx, float ny, float nz, float d) : mData(nx, ny, nz, d) {}
    explicit Plane(const glm::vec4& data) : mData(data) {}
    
    static Plane fromNormalAndPoint(const glm::vec3& normal, const glm::vec3& point) 
    {
        glm::vec3 normalizedNormal = glm::normalize(normal);
        float d = -glm::dot(normalizedNormal, point);
        return Plane(normalizedNormal.x, normalizedNormal.y, normalizedNormal.z, d);
    }
    
    static Plane fromPoints(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) 
    {
        glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        return fromNormalAndPoint(normal, a);
    }
    
    glm::vec3 getNormal() const 
    {
        return glm::vec3(mData.x, mData.y, mData.z);
    }
};

struct Triangle {
    Point3D vertices[3];  ///< The three vertices of the triangle
    
    Triangle() {}
    Triangle(const Point3D& a, const Point3D& b, const Point3D& c) 
    {
        vertices[0] = a;
        vertices[1] = b;
        vertices[2] = c;
    }
    
    glm::vec3 computeNormal() const 
    {
        glm::vec3 edge1 = vertices[1].mData - vertices[0].mData;
        glm::vec3 edge2 = vertices[2].mData - vertices[0].mData;
        return glm::normalize(glm::cross(edge1, edge2));
    }
};

struct Ray 
{
    glm::vec3 origin;     ///< Origin point of the ray
    glm::vec3 direction;  ///< Direction vector of the ray (normalized)
    
    Ray() : origin(0.0f), direction(0.0f, 0.0f, 1.0f) {}
    Ray(const glm::vec3& o, const glm::vec3& d)
        : origin(o), direction(glm::normalize(d)) {}
    
    glm::vec3 getPoint(float t) const {
        return origin + direction * t;
    }
};

struct BoundingSphere 
{
    glm::vec3 center;  ///< Center of the sphere
    float radius;      ///< Radius of the sphere
    
    BoundingSphere() : center(0.0f), radius(0.0f) {}
    BoundingSphere(const glm::vec3& c, float r) : center(c), radius(r) {}
    
    static BoundingSphere fromPoints(const std::vector<glm::vec3>& points);
};

struct AABB 
{
    glm::vec3 mCenter;       ///< Center of the box
    glm::vec3 mHalfExtents;  ///< Half-extents of the box along each axis
    
    AABB() : mCenter(0.0f), mHalfExtents(0.0f) {}
    AABB(const glm::vec3& center, const glm::vec3& halfExtents)
        : mCenter(center), mHalfExtents(halfExtents) {}
    
    static AABB fromMinMax(const glm::vec3& min, const glm::vec3& max) 
    {
        glm::vec3 center = (min + max) * 0.5f;
        glm::vec3 halfExtents = (max - min) * 0.5f;
        return AABB(center, halfExtents);
    }
    
    static AABB fromPoints(const std::vector<glm::vec3>& points);
    
    glm::vec3 getMin() const 
    {
        return mCenter - mHalfExtents;
    }
    
    glm::vec3 getMax() const 
    {
        return mCenter + mHalfExtents;
    }
}; 