#pragma once

#include <glm/glm.hpp>
#include <vector>

/**
 * @struct Point3D
 * @brief Represents a 3D point in space
 */
struct Point3D {
    glm::vec3 mData;  ///< Position data (x, y, z)

    Point3D() : mData(0.0f) {}
    Point3D(float x, float y, float z) : mData(x, y, z) {}
    explicit Point3D(const glm::vec3& data) : mData(data) {}
};

/**
 * @struct Plane
 * @brief Represents a plane in 3D space using the equation: nx*x + ny*y + nz*z + d = 0
 */
struct Plane {
    glm::vec4 mData;  ///< Plane equation coefficients (nx, ny, nz, d)

    Plane() : mData(0.0f, 0.0f, 0.0f, 0.0f) {}
    Plane(float nx, float ny, float nz, float d) : mData(nx, ny, nz, d) {}
    explicit Plane(const glm::vec4& data) : mData(data) {}
    
    /**
     * @brief Creates a plane from a normal vector and a point on the plane
     * @param normal The normal vector to the plane
     * @param point A point on the plane
     * @return A plane object
     */
    static Plane fromNormalAndPoint(const glm::vec3& normal, const glm::vec3& point) {
        glm::vec3 normalizedNormal = glm::normalize(normal);
        float d = -glm::dot(normalizedNormal, point);
        return Plane(normalizedNormal.x, normalizedNormal.y, normalizedNormal.z, d);
    }
    
    /**
     * @brief Creates a plane from three points
     * @param a First point
     * @param b Second point
     * @param c Third point
     * @return A plane object
     */
    static Plane fromPoints(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        return fromNormalAndPoint(normal, a);
    }
    
    /**
     * @brief Gets the normal vector of the plane
     * @return The normal vector
     */
    glm::vec3 getNormal() const {
        return glm::vec3(mData.x, mData.y, mData.z);
    }
};

/**
 * @struct Triangle
 * @brief Represents a triangle in 3D space
 */
struct Triangle {
    Point3D vertices[3];  ///< The three vertices of the triangle
    
    Triangle() {}
    Triangle(const Point3D& a, const Point3D& b, const Point3D& c) {
        vertices[0] = a;
        vertices[1] = b;
        vertices[2] = c;
    }
    
    /**
     * @brief Computes the normal vector for this triangle
     * @return The normal vector
     */
    glm::vec3 computeNormal() const {
        glm::vec3 edge1 = vertices[1].mData - vertices[0].mData;
        glm::vec3 edge2 = vertices[2].mData - vertices[0].mData;
        return glm::normalize(glm::cross(edge1, edge2));
    }
};

/**
 * @struct Ray
 * @brief Represents a ray in 3D space with an origin and direction
 */
struct Ray {
    glm::vec3 origin;     ///< Origin point of the ray
    glm::vec3 direction;  ///< Direction vector of the ray (normalized)
    
    Ray() : origin(0.0f), direction(0.0f, 0.0f, 1.0f) {}
    Ray(const glm::vec3& o, const glm::vec3& d)
        : origin(o), direction(glm::normalize(d)) {}
    
    /**
     * @brief Gets the point along the ray at a specific distance
     * @param t The distance along the ray
     * @return The point at distance t
     */
    glm::vec3 getPoint(float t) const {
        return origin + direction * t;
    }
};

/**
 * @struct BoundingSphere
 * @brief Represents a bounding sphere with a center point and radius
 */
struct BoundingSphere {
    glm::vec3 center;  ///< Center of the sphere
    float radius;      ///< Radius of the sphere
    
    BoundingSphere() : center(0.0f), radius(0.0f) {}
    BoundingSphere(const glm::vec3& c, float r) : center(c), radius(r) {}
    
    /**
     * @brief Creates a bounding sphere from a set of points
     * @param points Vector of points to be enclosed
     * @return A bounding sphere that encloses all points
     */
    static BoundingSphere fromPoints(const std::vector<glm::vec3>& points);
};

/**
 * @struct AABB
 * @brief Represents an axis-aligned bounding box
 */
struct AABB {
    glm::vec3 mCenter;       ///< Center of the box
    glm::vec3 mHalfExtents;  ///< Half-extents of the box along each axis
    
    AABB() : mCenter(0.0f), mHalfExtents(0.0f) {}
    AABB(const glm::vec3& center, const glm::vec3& halfExtents)
        : mCenter(center), mHalfExtents(halfExtents) {}
    
    /**
     * @brief Creates an AABB from minimum and maximum corners
     * @param min Minimum corner (smallest x, y, z values)
     * @param max Maximum corner (largest x, y, z values)
     * @return An AABB object
     */
    static AABB fromMinMax(const glm::vec3& min, const glm::vec3& max) {
        glm::vec3 center = (min + max) * 0.5f;
        glm::vec3 halfExtents = (max - min) * 0.5f;
        return AABB(center, halfExtents);
    }
    
    /**
     * @brief Creates an AABB from a set of points
     * @param points Vector of points to be enclosed
     * @return An AABB that encloses all points
     */
    static AABB fromPoints(const std::vector<glm::vec3>& points);
    
    /**
     * @brief Gets the minimum corner of the AABB
     * @return The minimum corner (smallest x, y, z values)
     */
    glm::vec3 getMin() const {
        return mCenter - mHalfExtents;
    }
    
    /**
     * @brief Gets the maximum corner of the AABB
     * @return The maximum corner (largest x, y, z values)
     */
    glm::vec3 getMax() const {
        return mCenter + mHalfExtents;
    }
}; 