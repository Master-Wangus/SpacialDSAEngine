/**
 * @class CameraSystem
 * @brief System for handling camera movement and control for multiple camera types.
 *
 * This system manages camera movement, rotation, and view calculations for both
 * first-person and orbital perspectives, responding to user input and providing smooth camera dynamics.
 */

#pragma once

#include "pch.h"

// Forward declarations
class Registry;
class Window;
struct CameraComponent;

enum class CameraType 
{
    FPS,
    Orbital
    // More incoming!
};

struct Projection 
{
    float m_Fov = 45.0f;
    float m_NearPlane = 0.1f;
    float m_FarPlane = 100.0f;
    
    Projection() = default;
    Projection(float fov, float nearPlane, float farPlane)
        : m_Fov(fov), m_NearPlane(nearPlane), m_FarPlane(farPlane) {}
        
    glm::mat4 GetProjectionMatrix(float aspectRatio) const 
    {
        return glm::perspective(glm::radians(m_Fov), aspectRatio, m_NearPlane, m_FarPlane);
    }
};

struct FPSCamera 
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

    FPSCamera(
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

struct OrbitalCamera 
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

    OrbitalCamera(
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

class CameraSystem 
{
public:
    CameraSystem(Registry& registry, Window& window);
    
    void OnRun(float deltaTime);
    
    glm::mat4 GetViewMatrix(const CameraComponent& camera);

private:
    void SetupInputCallbacks();
    void ProcessMouseMovement();
    void ProcessKeyboardInput(float deltaTime);
    void SwitchCameraType();
    
    Registry& m_Registry;
    Window& m_Window;
    Registry::Entity m_CameraEntity = entt::null;
    
    // Camera switching state
    bool m_CKeyPressed = false;
}; 