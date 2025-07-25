/**
 * @file Components.hpp
 * @brief Component definitions for the entity-component system.
 *
 * This file contains the definitions of all components used in the entity-component system,
 * including transform, rendering, collision, camera, and lighting components.
 */

#pragma once

#include "pch.h"
#include "Primitives.hpp"
#include "Lighting.hpp"
#include "Buffer.hpp"
#include "IRenderable.hpp"

class Shader;

// ==================== Transform Components ====================

struct TransformComponent 
{
    glm::vec3 m_Position;
    glm::vec3 m_Rotation;
    glm::vec3 m_Scale;
    glm::mat4 m_Model;
    
    TransformComponent(
        const glm::vec3& pos = glm::vec3(0.0f),
        const glm::vec3& rot = glm::vec3(0.0f),
        const glm::vec3& scl = glm::vec3(1.0f))
        : m_Position(pos), m_Rotation(rot), m_Scale(scl), m_Model(1.0f)
    {
        UpdateModelMatrix();
    }
    
    void UpdateModelMatrix();
};

// ==================== Rendering Components ====================

struct RenderComponent 
{
    std::shared_ptr<IRenderable> m_Renderable;
    
    RenderComponent() = default;
    explicit RenderComponent(std::shared_ptr<IRenderable> r) : m_Renderable(std::move(r)) {}
};

// ==================== Physics Components ====================

// Collision shape types supported by the CollisionComponent
enum class CollisionShapeType 
{
    None,
    Sphere,
    AABB,
    Plane,
    Triangle,
    Ray,
    Point
};

struct CollisionComponent 
{
    CollisionShapeType m_ShapeType = CollisionShapeType::None;
    
    // Shape data
    union 
    {
        struct 
        {
            glm::vec3 m_Center;
            float m_Radius; 
        } m_Sphere;
        
        struct 
        {
            glm::vec3 m_Center;
            glm::vec3 m_HalfExtents;
        } m_AABB;
        
        struct 
        {
            glm::vec3 m_Normal;
            float m_Distance;
        } m_Plane;
        
        struct 
        {
            glm::vec3 m_Vertices[3];
            glm::vec3 m_LocalVertices[3];  // Store local space vertices for proper transformation
        } m_Triangle;
        
        struct 
        {
            glm::vec3 m_Origin;
            glm::vec3 m_Direction;
            float m_Length;
        } m_Ray;
        
        struct 
        {
            glm::vec3 m_Position;
        } m_Point;
    };
    
    // How does one initialize unions???
    CollisionComponent() = default;
    
    // Sphere constructor
    static CollisionComponent CreateSphere(const glm::vec3& center, float radius) 
    {
        CollisionComponent comp;
        comp.m_ShapeType = CollisionShapeType::Sphere;
        comp.m_Sphere.m_Center = center;
        comp.m_Sphere.m_Radius = radius;
        return comp;
    }
    
    // AABB constructor
    static CollisionComponent CreateAABB(const glm::vec3& center, const glm::vec3& halfExtents) 
    {
        CollisionComponent comp;
        comp.m_ShapeType = CollisionShapeType::AABB;
        comp.m_AABB.m_Center = center;
        comp.m_AABB.m_HalfExtents = halfExtents;
        return comp;
    }
    
    // Plane constructor
    static CollisionComponent CreatePlane(const glm::vec3& normal, float distance) 
    {
        CollisionComponent comp;
        comp.m_ShapeType = CollisionShapeType::Plane;
        comp.m_Plane.m_Normal = glm::normalize(normal);
        comp.m_Plane.m_Distance = distance;
        return comp;
    }
    
    // Triangle constructor
    static CollisionComponent CreateTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) 
    {
        CollisionComponent comp;
        comp.m_ShapeType = CollisionShapeType::Triangle;
        comp.m_Triangle.m_Vertices[0] = v0;
        comp.m_Triangle.m_Vertices[1] = v1;
        comp.m_Triangle.m_Vertices[2] = v2;
        
        // Calculate center and store local vertices
        glm::vec3 center = (v0 + v1 + v2) / 3.0f;
        comp.m_Triangle.m_LocalVertices[0] = v0 - center;
        comp.m_Triangle.m_LocalVertices[1] = v1 - center;
        comp.m_Triangle.m_LocalVertices[2] = v2 - center;
        
        return comp;
    }
    
    // Ray constructor
    static CollisionComponent CreateRay(const glm::vec3& origin, const glm::vec3& direction, float length = 100.0f) 
    {
        CollisionComponent comp;
        comp.m_ShapeType = CollisionShapeType::Ray;
        comp.m_Ray.m_Origin = origin;
        comp.m_Ray.m_Direction = glm::normalize(direction);
        comp.m_Ray.m_Length = length;
        return comp;
    }
    
    // Point constructor
    static CollisionComponent CreatePoint(const glm::vec3& position) 
    {
        CollisionComponent comp;
        comp.m_ShapeType = CollisionShapeType::Point;
        comp.m_Point.m_Position = position;
        return comp;
    }
    
    // Conversion from primitive objects
    static CollisionComponent FromBoundingSphere(const BoundingSphere& sphere) 
    {
        return CreateSphere(sphere.m_Center, sphere.m_Radius);
    }
    
    static CollisionComponent FromAABB(const AABB& aabb) 
    {
        return CreateAABB(aabb.m_Center, aabb.m_HalfExtents);
    }
    
    // Update the transform of the collision shape
    void UpdateTransform(const glm::vec3& position, const glm::vec3& scale) 
    {
        switch (m_ShapeType) 
        {
            case CollisionShapeType::Sphere:
                m_Sphere.m_Center = position;
                m_Sphere.m_Radius = glm::max(glm::max(scale.x, scale.y), scale.z);
                break;
                
            case CollisionShapeType::AABB:
                m_AABB.m_Center = position;
                m_AABB.m_HalfExtents = scale * 0.5f;
                break;
                
            case CollisionShapeType::Point:
                m_Point.m_Position = position;
                break;
                
            case CollisionShapeType::Ray:
                m_Ray.m_Origin = position;
                break;
                
            case CollisionShapeType::Plane:
            {
                float signedDist = glm::dot(m_Plane.m_Normal, position);
                m_Plane.m_Distance = -signedDist; // Plane equation: dot(normal, point) + distance = 0
                break;
            }
                
            case CollisionShapeType::Triangle:
            {
                // Transform local vertices to world space using the new position
                m_Triangle.m_Vertices[0] = position + m_Triangle.m_LocalVertices[0];
                m_Triangle.m_Vertices[1] = position + m_Triangle.m_LocalVertices[1];
                m_Triangle.m_Vertices[2] = position + m_Triangle.m_LocalVertices[2];
                break;
            }
                
            default:
                break;
        }
    }
};

// ==================== Lighting Components ====================

struct DirectionalLightComponent 
{
    DirectionalLight m_Light;
    
    DirectionalLightComponent() = default;
    DirectionalLightComponent(const DirectionalLight& l) : m_Light(l) {}
};

// ==================== Camera Components ====================

enum class CameraType 
{
    FPS,
    Orbital
    // More incoming!
};

struct ProjectionComponent 
{
    float m_Fov = 45.0f;
    float m_NearPlane = 0.1f;
    float m_FarPlane = 100.0f;
    
    ProjectionComponent() = default;
    ProjectionComponent(float fov, float nearPlane, float farPlane)
        : m_Fov(fov), m_NearPlane(nearPlane), m_FarPlane(farPlane) {}
        
    glm::mat4 GetProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(m_Fov), aspectRatio, m_NearPlane, m_FarPlane);
    }
};

struct FPSCameraComponent 
{
    glm::vec3 m_CameraPosition;
    glm::vec3 m_CameraFront;
    glm::vec3 m_CameraUpDirection;
    float m_YawAngle;
    float m_PitchAngle;
    float m_MovementSpeed;
    float m_MovementAcceleration;
    float m_DragFriction;
    glm::vec3 m_CurrentVelocity;
    float m_MouseSensitivity;

    FPSCameraComponent(
        const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f),
        const glm::vec3& front = glm::vec3(0.0f, 0.0f, -1.0f),
        const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f))
        : m_CameraPosition(position),
          m_CameraFront(front),
          m_CameraUpDirection(up),
          m_YawAngle(-90.0f),  // Default looking forward
          m_PitchAngle(0.0f),
          m_MovementSpeed(2.5f),
          m_MovementAcceleration(10.0f),
          m_DragFriction(5.0f),
          m_CurrentVelocity(0.0f),
          m_MouseSensitivity(0.1f)
    {
        UpdateVectors();
    }

    void UpdateVectors() 
    {
        // Calculate front vector from yaw and pitch
        glm::vec3 front;
        front.x = cos(glm::radians(m_YawAngle)) * cos(glm::radians(m_PitchAngle));
        front.y = sin(glm::radians(m_PitchAngle));
        front.z = sin(glm::radians(m_YawAngle)) * cos(glm::radians(m_PitchAngle));
        m_CameraFront = glm::normalize(front);
    }

    glm::mat4 GetViewMatrix() const 
    {
        return glm::lookAt(m_CameraPosition, m_CameraPosition + m_CameraFront, m_CameraUpDirection);
    }
};

struct OrbitalCameraComponent 
{
    glm::vec3 m_Target;           // Point the camera orbits around
    float m_Distance;             // Distance from target
    float m_Yaw;                  // Horizontal rotation (around Y-axis)
    float m_Pitch;                // Vertical rotation
    float m_MinDistance;          // Minimum zoom distance
    float m_MaxDistance;          // Maximum zoom distance
    float m_ZoomSpeed;            // Speed of zooming
    float m_OrbitSpeed;           // Speed of orbital movement
    float m_MouseSensitivity;     // Mouse sensitivity for orbital control

    OrbitalCameraComponent(
        const glm::vec3& target = glm::vec3(0.0f, 0.0f, 0.0f),
        float distance = 5.0f,
        float yaw = 0.0f,
        float pitch = 0.0f)
        : m_Target(target),
          m_Distance(distance),
          m_Yaw(yaw),
          m_Pitch(pitch),
          m_MinDistance(1.0f),
          m_MaxDistance(50.0f),
          m_ZoomSpeed(2.0f),
          m_OrbitSpeed(1.0f),
          m_MouseSensitivity(0.5f)
    {
        // Clamp pitch to prevent flipping
        m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
        m_Distance = glm::clamp(m_Distance, m_MinDistance, m_MaxDistance);
    }

    glm::vec3 GetCameraPosition() const 
    {
        // Convert spherical coordinates to cartesian
        float x = m_Distance * cos(glm::radians(m_Pitch)) * cos(glm::radians(m_Yaw));
        float y = m_Distance * sin(glm::radians(m_Pitch));
        float z = m_Distance * cos(glm::radians(m_Pitch)) * sin(glm::radians(m_Yaw));
        return m_Target + glm::vec3(x, y, z);
    }

    glm::mat4 GetViewMatrix() const 
    {
        glm::vec3 cameraPos = GetCameraPosition();
        return glm::lookAt(cameraPos, m_Target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void Zoom(float deltaZoom) 
    {
        m_Distance += deltaZoom * m_ZoomSpeed;
        m_Distance = glm::clamp(m_Distance, m_MinDistance, m_MaxDistance);
    }

    void Orbit(float deltaYaw, float deltaPitch) 
    {
        m_Yaw += deltaYaw * m_MouseSensitivity * m_OrbitSpeed;
        m_Pitch += deltaPitch * m_MouseSensitivity * m_OrbitSpeed;
        
        // Keep yaw in [0, 360) range
        if (m_Yaw > 360.0f) m_Yaw -= 360.0f;
        if (m_Yaw < 0.0f) m_Yaw += 360.0f;
        
        // Clamp pitch to prevent flipping
        m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
    }
};

struct CameraComponent 
{
    ProjectionComponent m_Projection;
    FPSCameraComponent m_FPS;
    OrbitalCameraComponent m_Orbital;
    CameraType m_ActiveCameraType = CameraType::FPS;
    
    CameraComponent() = default;
    CameraComponent(
        const ProjectionComponent& proj,
        const FPSCameraComponent& fpsCamera,
        const OrbitalCameraComponent& orbitalCamera = OrbitalCameraComponent(),
        CameraType type = CameraType::FPS)
        : m_Projection(proj), m_FPS(fpsCamera), m_Orbital(orbitalCamera), m_ActiveCameraType(type) {}
    
    glm::mat4 GetViewMatrix() const 
    {
        switch (m_ActiveCameraType) 
        {
            case CameraType::FPS:
                return m_FPS.GetViewMatrix();
            case CameraType::Orbital:
                return m_Orbital.GetViewMatrix();
            default:
                return m_FPS.GetViewMatrix(); // Default to FPS camera
        }
    }
    
    glm::mat4 GetProjectionMatrix(float aspectRatio) const 
    {
        return m_Projection.GetProjectionMatrix(aspectRatio);
    }
    
    glm::vec3 GetPosition() const 
    {
        switch (m_ActiveCameraType) 
        {
            case CameraType::FPS:
                return m_FPS.m_CameraPosition;
            case CameraType::Orbital:
                return m_Orbital.GetCameraPosition();
            default:
                return m_FPS.m_CameraPosition; // Default to FPS camera
        }
    }
};
