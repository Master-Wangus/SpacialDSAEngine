#pragma once

#include <glm/glm.hpp>
#include <vector>


struct Point3D 
{
    glm::vec3 m_Data;  ///< Position data (x, y, z)

    Point3D() : m_Data(0.0f) {}
    Point3D(float x, float y, float z) : m_Data(x, y, z) {}
    explicit Point3D(const glm::vec3& data) : m_Data(data) {}
};


struct Plane 
{
    glm::vec4 m_Data;  ///< Plane equation coefficients (nx, ny, nz, d)

    Plane() : m_Data(0.0f, 0.0f, 0.0f, 0.0f) {}
    Plane(float nx, float ny, float nz, float d) : m_Data(nx, ny, nz, d) {}
    explicit Plane(const glm::vec4& data) : m_Data(data) {}
    
    static Plane FromNormalAndPoint(const glm::vec3& normal, const glm::vec3& point) 
    {
        glm::vec3 normalizedNormal = glm::normalize(normal);
        float d = -glm::dot(normalizedNormal, point);
        return Plane(normalizedNormal.x, normalizedNormal.y, normalizedNormal.z, d);
    }
    
    static Plane FromPoints(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) 
    {
        glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        return FromNormalAndPoint(normal, a);
    }
    
    glm::vec3 GetNormal() const 
    {
        return glm::vec3(m_Data.x, m_Data.y, m_Data.z);
    }
};

struct Triangle {
    Point3D m_Vertices[3];  ///< The three vertices of the triangle
    
    Triangle() {}
    Triangle(const Point3D& a, const Point3D& b, const Point3D& c) 
    {
        m_Vertices[0] = a;
        m_Vertices[1] = b;
        m_Vertices[2] = c;
    }
    
    glm::vec3 ComputeNormal() const 
    {
        glm::vec3 edge1 = m_Vertices[1].m_Data - m_Vertices[0].m_Data;
        glm::vec3 edge2 = m_Vertices[2].m_Data - m_Vertices[0].m_Data;
        return glm::normalize(glm::cross(edge1, edge2));
    }
};

struct Ray 
{
    glm::vec3 m_Origin;     ///< Origin point of the ray
    glm::vec3 m_Direction;  ///< Direction vector of the ray (normalized)
    
    Ray() : m_Origin(0.0f), m_Direction(0.0f, 0.0f, 1.0f) {}
    Ray(const glm::vec3& o, const glm::vec3& d)
        : m_Origin(o), m_Direction(glm::normalize(d)) {}
    
    glm::vec3 GetPoint(float t) const {
        return m_Origin + m_Direction * t;
    }
};

struct BoundingSphere 
{
    glm::vec3 m_Center;  ///< Center of the sphere
    float m_Radius;      ///< Radius of the sphere
    
    BoundingSphere() : m_Center(0.0f), m_Radius(0.0f) {}
    BoundingSphere(const glm::vec3& c, float r) : m_Center(c), m_Radius(r) {}
    
    static BoundingSphere FromPoints(const std::vector<glm::vec3>& points);
};

struct AABB 
{
    glm::vec3 m_Center;       ///< Center of the box
    glm::vec3 m_HalfExtents;  ///< Half-extents of the box along each axis
    
    AABB() : m_Center(0.0f), m_HalfExtents(0.0f) {}
    AABB(const glm::vec3& center, const glm::vec3& halfExtents)
        : m_Center(center), m_HalfExtents(halfExtents) {}
    
    static AABB FromMinMax(const glm::vec3& min, const glm::vec3& max);
    
    static AABB FromPoints(const std::vector<glm::vec3>& points);
    
    glm::vec3 GetMin() const 
    {
        return m_Center - m_HalfExtents;
    }
    
    glm::vec3 GetMax() const 
    {
        return m_Center + m_HalfExtents;
    }
}; 