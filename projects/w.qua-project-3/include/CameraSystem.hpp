/**
 * @class CameraSystem
 * @brief System for handling camera movement and control for multiple camera types.
 *
 * This system manages camera movement, rotation, and view calculations for both
 * first-person and orbital perspectives, responding to user input and providing smooth camera dynamics.
 */

#pragma once

#include "pch.h"
#include "Shapes.hpp" // Added for bounding volume testing
#include "Geometry.hpp" // Include for SideResult enum

class Registry;
class Window;
struct CameraComponent;
enum class SideResult;

// Frustum culling result colors
const glm::vec3 FRUSTUM_INSIDE_COLOR = glm::vec3(0.0f, 1.0f, 0.0f);      // Green
const glm::vec3 FRUSTUM_OUTSIDE_COLOR = glm::vec3(1.0f, 0.0f, 0.0f);     // Red
const glm::vec3 FRUSTUM_INTERSECT_COLOR = glm::vec3(1.0f, 1.0f, 0.0f);   // Yellow

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
    
    /**
     * @brief Constructs a projection with specified parameters.
     * @param fov Field of view in degrees
     * @param nearPlane Near clipping plane distance
     * @param farPlane Far clipping plane distance
     */
    Projection(float fov, float nearPlane, float farPlane)
        : m_Fov(fov), m_NearPlane(nearPlane), m_FarPlane(farPlane) {}
        
    /**
     * @brief Gets the projection matrix for the given aspect ratio.
     * @param aspectRatio Viewport aspect ratio (width/height)
     * @return 4x4 projection matrix
     */
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

    /**
     * @brief Constructs an FPS camera with specified parameters.
     * @param position Initial camera position
     * @param front Initial front vector direction
     * @param up Initial up vector direction
     */
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

    /**
     * @brief Updates camera direction vectors based on yaw and pitch angles.
     */
    void UpdateVectors() 
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_YawAngle)) * cos(glm::radians(m_PitchAngle));
        front.y = sin(glm::radians(m_PitchAngle));
        front.z = sin(glm::radians(m_YawAngle)) * cos(glm::radians(m_PitchAngle));
        m_CameraFront = glm::normalize(front);
    }

    /**
     * @brief Gets the view matrix for this FPS camera.
     * @return 4x4 view matrix
     */
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

    /**
     * @brief Constructs an orbital camera with specified parameters.
     * @param target Point to orbit around
     * @param distance Initial distance from target
     * @param yaw Initial horizontal rotation angle
     * @param pitch Initial vertical rotation angle
     */
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
        m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
        m_Distance = glm::clamp(m_Distance, m_MinDistance, m_MaxDistance);
    }

    /**
     * @brief Calculates the current camera position based on orbital parameters.
     *        (Top-down debug camera: camera sits above the target at a fixed height)
     * @return Current camera position in world space
     */
    glm::vec3 GetCameraPosition() const
    {
        // For a top-down debug view we always stay directly above the target.
        return m_Target + glm::vec3(0.0f, m_Distance, 0.0f);
    }

    /**
     * @brief Gets the view matrix for this orbital camera.
     * @return 4x4 view matrix
     */
    glm::mat4 GetViewMatrix() const
    {
        glm::vec3 cameraPos = GetCameraPosition();
        // Use -Z as the up vector to keep world X axis to the right on screen.
        return glm::lookAt(cameraPos, m_Target, glm::vec3(0.0f, 0.0f, -1.0f));
    }

    /**
     * @brief Adjusts the distance from the target (zoom).
     * @param deltaZoom Amount to zoom (positive = zoom out, negative = zoom in)
     */
    void Zoom(float deltaZoom) 
    {
        m_Distance += deltaZoom * m_ZoomSpeed;
        m_Distance = glm::clamp(m_Distance, m_MinDistance, m_MaxDistance);
    }

    /**
     * @brief Rotates the camera around the target.
     * @param deltaYaw Change in horizontal rotation
     * @param deltaPitch Change in vertical rotation
     */
    void Orbit(float deltaYaw, float deltaPitch) 
    {
        m_Yaw += deltaYaw * m_MouseSensitivity * m_OrbitSpeed;
        m_Pitch += deltaPitch * m_MouseSensitivity * m_OrbitSpeed;
        
        // Keep yaw in [0, 360) range
        if (m_Yaw > 360.0f) m_Yaw -= 360.0f;
        else if (m_Yaw < 0.0f) m_Yaw += 360.0f;
        
        // Clamp pitch to prevent flipping
        m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
    }

    /**
     * @brief Moves the target point.
     * @param offset Direction and magnitude to move target
     */
    void MoveTarget(const glm::vec3& offset) 
    {
        m_Target += offset;
    }
};

class CameraSystem 
{
public:
    /**
     * @brief Constructs a camera system with registry and window dependencies.
     * @param registry Entity registry for camera components
     * @param window Window reference for viewport aspect ratio
     */
    CameraSystem(Registry& registry, Window& window);
    
    /**
     * @brief Destructor for the camera system.
     */
    ~CameraSystem();
    
    /**
     * @brief Updates camera state each frame.
     * @param deltaTime Time elapsed since the last frame
     */
    void OnRun(float deltaTime);
    
    /**
     * @brief Gets the view matrix for the given camera.
     * @param camera Camera component to get view matrix from
     * @return 4x4 view matrix
     */
    glm::mat4 GetViewMatrix(const CameraComponent& camera);
    
    /**
     * @brief Updates the frustum culling planes for a camera.
     * @param camera Camera component to update frustum for
     * @param aspectRatio Viewport aspect ratio
     */
    void UpdateFrustumPlanes(const CameraComponent& camera, float aspectRatio);
    
    /**
     * @brief Tests if a sphere is inside, outside, or intersecting the frustum.
     * @param sphere Sphere to test
     * @return Classification result
     */
    SideResult TestSphereAgainstFrustum(const Sphere& sphere) const;
    
    /**
     * @brief Tests if an AABB is inside, outside, or intersecting the frustum.
     * @param aabb AABB to test
     * @return Classification result
     */
    SideResult TestAabbAgainstFrustum(const Aabb& aabb) const;
    
    /**
     * @brief Tests if an OBB is inside, outside, or intersecting the frustum.
     * @param obb OBB to test
     * @return Classification result
     */
    SideResult TestObbAgainstFrustum(const Obb& obb) const;
    
    /**
     * @brief Gets the appropriate color for visualizing frustum test results.
     * @param result Result of frustum test
     * @return RGB color vector
     */
    glm::vec3 GetFrustumTestColor(SideResult result) const;
    
    /**
     * @brief Gets the view-projection matrix for visualization.
     * @param camera Camera component to get view-projection from
     * @param aspectRatio Viewport aspect ratio
     * @return 4x4 view-projection matrix
     */
    glm::mat4 GetVisualizationViewProjectionMatrix(const CameraComponent& camera, float aspectRatio) const;
    
    /**
     * @brief Sets the reference camera projection used for visualization.
     * @param projection Projection parameters
     */
    void SetReferenceCameraProjection(const Projection& projection) { m_ReferenceCameraProjection = projection; }
    
    /**
     * @brief Gets the reference camera projection used for visualization.
     * @return Reference camera projection
     */
    Projection GetReferenceCameraProjection() const { return m_ReferenceCameraProjection; }

private:
    // Setup and processing methods
    void SetupEventSubscriptions();
    void ProcessMouseMovement(const EventData& eventData);
    void ProcessKeyboardInput(float deltaTime);
    void HandleMouseButtonPressEvent(const EventData& eventData);
    void HandleMouseButtonReleaseEvent(const EventData& eventData);
    void HandleMouseScrollEvent(const EventData& eventData);
    void HandleKeyPressEvent(const EventData& eventData);
    void HandleKeyReleaseEvent(const EventData& eventData);
    void SwitchCameraType();
    
    Registry& m_Registry;
    Window& m_Window;
    
    Registry::Entity m_CameraEntity = entt::null;
    
    bool m_CKeyPressed = false;
    bool m_MouseDragging = false;
    glm::vec2 m_LastMousePos = glm::vec2(0.0f);
    
    // Frustum culling data
    glm::vec3 m_FrustumNormals[6];
    float m_FrustumDistances[6];
    bool m_FrustumUpdated = false; // Flag to track if frustum needs updating
    
    Projection m_ReferenceCameraProjection;

    // --- Stored FPS state when switching to Top-Down camera ---
    bool m_HasStoredFPSState = false;
    glm::vec3 m_StoredFPSPosition = glm::vec3(0.0f);
    float m_StoredFPSYaw = 0.0f;
    float m_StoredFPSPitch = 0.0f;

    // Stored top-down (orbital) state
    bool  m_HasStoredTDState = false;
    glm::vec3 m_StoredTDTarget = glm::vec3(0.0f);
    float      m_StoredTDDistance = 10.0f;
}; 