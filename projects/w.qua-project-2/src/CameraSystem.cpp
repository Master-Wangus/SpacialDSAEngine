/**
 * @class CameraSystem
 * @brief System for handling camera movement and control for multiple camera types.
 *
 * This system manages camera movement, rotation, and view calculations for both
 * first-person and orbital perspectives, responding to user input and providing smooth camera dynamics.
 */

#include "pch.h"
#include "CameraSystem.hpp"
#include "Window.hpp"
#include "InputSystem.hpp"
#include "Systems.hpp"
#include "Keybinds.hpp"
#include "Registry.hpp"
#include "Components.hpp"
#include "Geometry.hpp" // Added for frustum culling functions

CameraSystem::CameraSystem(Registry& registry, Window& window)
    : m_Registry(registry), m_Window(window)
{
    auto cameraView = registry.View<CameraComponent>();
    if (!cameraView.empty()) {
        m_CameraEntity = *cameraView.begin();
    } 
    else 
    {
        
        m_CameraEntity = registry.Create();
        
        FPSCamera fpsCamera(
            glm::vec3(0.0f, 0.0f, 3.0f),  // position
            glm::vec3(0.0f, 0.0f, -1.0f), // front
            glm::vec3(0.0f, 1.0f, 0.0f)   // up
        );
        
        OrbitalCamera orbitalCamera(
            glm::vec3(0.0f, 0.0f, 0.0f),  // target
            5.0f,                          // distance
            45.0f,                         // yaw
            30.0f                          // pitch
        );
        
        Projection projection(
            45.0f,  // fov
            0.1f,   // near plane
            100.0f  // far plane
        );
        
        CameraComponent camera;
        camera.m_FPS = fpsCamera;
        camera.m_Orbital = orbitalCamera;
        camera.m_Projection = projection;
        camera.m_ActiveCameraType = CameraType::FPS;
        
        registry.AddComponent<CameraComponent>(m_CameraEntity, camera);
    }
    
    // Initialize frustum planes
    for (int i = 0; i < 6; ++i) {
        m_FrustumNormals[i] = glm::vec3(0.0f);
        m_FrustumDistances[i] = 0.0f;
    }
    
    // Initialize reference camera projection with default values
    m_ReferenceCameraProjection = Projection(60.0f, 0.5f, 1000.0f);
    
    // Set up input callbacks for this system
    SetupInputCallbacks();
}

CameraSystem::~CameraSystem()
{
    // No cleanup needed
}

void CameraSystem::SetupInputCallbacks()
{
    // Subscribe to mouse movement
    Systems::g_InputSystem->SubscribeToMouseMove([this](double xpos, double ypos) {
        if (Systems::g_InputSystem->IsMouseDragging()) 
        {
            this->ProcessMouseMovement();
        }
    });
    
    // Subscribe to mouse button for camera control
    Systems::g_InputSystem->SubscribeToMouseButton(Keybinds::MOUSE_BUTTON_RIGHT, 
        [this](int button, int action, int mods) {
            // Only handle right mouse button for camera control
            if (button == Keybinds::MOUSE_BUTTON_RIGHT)
            {
                if (action == Keybinds::PRESS)
                {
                    Systems::g_InputSystem->StartDragging();
                }
                else if (action == Keybinds::RELEASE)
                {
                    Systems::g_InputSystem->StopDragging();
                }
            }
        });
    
    // Subscribe to scroll for orbital camera zoom
    Systems::g_InputSystem->SubscribeToMouseScroll([this](double xoffset, double yoffset) {
        auto& camera = m_Registry.GetComponent<CameraComponent>(m_CameraEntity);
        if (camera.m_ActiveCameraType == CameraType::Orbital) {
            camera.m_Orbital.Zoom(-static_cast<float>(yoffset));
        }
    });
}

void CameraSystem::OnRun(float deltaTime)
{
    // Only process input if we have a valid camera entity
    auto cameraView = m_Registry.View<CameraComponent>();
    if (!cameraView.empty()) {
        ProcessKeyboardInput(deltaTime);
    }
}

glm::mat4 CameraSystem::GetViewMatrix(const CameraComponent& camera)
{
    return camera.GetViewMatrix();
}

void CameraSystem::ProcessMouseMovement()
{
    glm::vec2 mouseDelta = Systems::g_InputSystem->GetMouseDelta();
    
    // Skip if no movement
    if (glm::length(mouseDelta) < 0.0001f)
        return;
    
    float xOffset = mouseDelta.x;
    float yOffset = mouseDelta.y;
    
    auto& camera = m_Registry.GetComponent<CameraComponent>(m_CameraEntity);
    
    if (camera.m_ActiveCameraType == CameraType::FPS) 
    {
        const float sensitivity = 0.1f;
        xOffset *= sensitivity;
        yOffset *= sensitivity;
        
        camera.m_FPS.m_YawAngle += xOffset;
        camera.m_FPS.m_PitchAngle += yOffset;
        
        // Constrain pitch
        if (camera.m_FPS.m_PitchAngle > 89.0f) camera.m_FPS.m_PitchAngle = 89.0f;
        if (camera.m_FPS.m_PitchAngle < -89.0f) camera.m_FPS.m_PitchAngle = -89.0f;
        
        camera.m_FPS.UpdateVectors();
    }
    else if (camera.m_ActiveCameraType == CameraType::Orbital) 
    {
        // For orbital camera, mouse movement rotates around the target
        camera.m_Orbital.Orbit(xOffset, -yOffset); // Negative yOffset for intuitive up/down movement
    }
}

void CameraSystem::ProcessKeyboardInput(float deltaTime)
{
    auto& camera = m_Registry.GetComponent<CameraComponent>(m_CameraEntity);
    
    bool cKeyCurrentlyPressed = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_C);
    if (cKeyCurrentlyPressed && !m_CKeyPressed) {
        SwitchCameraType();
    }
    m_CKeyPressed = cKeyCurrentlyPressed;
    
    if (camera.m_ActiveCameraType == CameraType::FPS) 
    {
        bool keyW = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_W);
        bool keyS = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_S);
        bool keyA = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_A);
        bool keyD = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_D);
        
        float velocity = camera.m_FPS.m_MovementSpeed * deltaTime;
        
        if (keyW)
            camera.m_FPS.m_CameraPosition += camera.m_FPS.m_CameraFront * velocity;
        if (keyS)
            camera.m_FPS.m_CameraPosition -= camera.m_FPS.m_CameraFront * velocity;
        if (keyA)
            camera.m_FPS.m_CameraPosition -= glm::normalize(glm::cross(camera.m_FPS.m_CameraFront, camera.m_FPS.m_CameraUpDirection)) * velocity;
        if (keyD)
            camera.m_FPS.m_CameraPosition += glm::normalize(glm::cross(camera.m_FPS.m_CameraFront, camera.m_FPS.m_CameraUpDirection)) * velocity;
    }
    else if (camera.m_ActiveCameraType == CameraType::Orbital) 
    {
        bool keyW = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_W);
        bool keyS = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_S);
        bool keyA = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_A);
        bool keyD = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_D);
        
        // Move target point in orbital mode
        float targetMoveSpeed = 5.0f * deltaTime;
        
        if (keyW)
            camera.m_Orbital.m_Target.z -= targetMoveSpeed;
        if (keyS)
            camera.m_Orbital.m_Target.z += targetMoveSpeed;
        if (keyA)
            camera.m_Orbital.m_Target.x -= targetMoveSpeed;
        if (keyD)
            camera.m_Orbital.m_Target.x += targetMoveSpeed;
    }
}

void CameraSystem::SwitchCameraType()
{
    auto& camera = m_Registry.GetComponent<CameraComponent>(m_CameraEntity);
    
    if (camera.m_ActiveCameraType == CameraType::FPS) 
    {
        // Set orbital target to current FPS camera position + front direction
        camera.m_Orbital.m_Target = camera.m_FPS.m_CameraPosition + camera.m_FPS.m_CameraFront * 3.0f;
        camera.m_ActiveCameraType = CameraType::Orbital;
    }
    else 
    {
        // Set FPS camera position to orbital camera position
        camera.m_FPS.m_CameraPosition = camera.m_Orbital.GetCameraPosition();
        camera.m_FPS.m_CameraFront = glm::normalize(camera.m_Orbital.m_Target - camera.m_FPS.m_CameraPosition);
        camera.m_FPS.UpdateVectors();
        camera.m_ActiveCameraType = CameraType::FPS;
    }
}

void CameraSystem::UpdateFrustumPlanes(const CameraComponent& camera, float aspectRatio)
{
    // Calculate view-projection matrix using the reference camera projection (not the main camera's projection)
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = m_ReferenceCameraProjection.GetProjectionMatrix(aspectRatio);
    glm::mat4 viewProjection = projection * view;
    
    // Extract frustum planes from the view-projection matrix
    FrustumFromVp(viewProjection, m_FrustumNormals, m_FrustumDistances);
    
    m_FrustumUpdated = true;
}

SideResult CameraSystem::TestSphereAgainstFrustum(const Sphere& sphere) const
{
    if (!m_FrustumUpdated) {
        return SideResult::eINSIDE; // Default to inside if frustum not updated
    }
    
    // Create a vertex to use with the classification function
    Vertex center;
    center.m_Position = sphere.center;
    
    // Test sphere against all frustum planes
    return ClassifyFrustumSphereNaive(m_FrustumNormals, m_FrustumDistances, center, sphere.radius);
}

SideResult CameraSystem::TestAabbAgainstFrustum(const Aabb& aabb) const
{
    if (!m_FrustumUpdated) {
        return SideResult::eINSIDE; // Default to inside if frustum not updated
    }
    
    // Create vertices for min and max points
    Vertex min, max;
    min.m_Position = aabb.min;
    max.m_Position = aabb.max;
    
    // Test AABB against all frustum planes
    return ClassifyFrustumAabbNaive(m_FrustumNormals, m_FrustumDistances, min, max);
}

SideResult CameraSystem::TestObbAgainstFrustum(const Obb& obb) const
{
    if (!m_FrustumUpdated) {
        return SideResult::eINSIDE; // Default to inside if frustum not updated
    }
    
    // Convert OBB to AABB in model space for testing
    // This is a simplification - for accurate OBB testing we would need additional functions
    
    // Calculate the 8 corners of the OBB
    glm::vec3 corners[8];
    for (int i = 0; i < 8; i++) {
        glm::vec3 direction(
            (i & 1) ? 1.0f : -1.0f,
            (i & 2) ? 1.0f : -1.0f,
            (i & 4) ? 1.0f : -1.0f
        );
        
        corners[i] = obb.center + 
            direction.x * obb.axes[0] * obb.halfExtents.x +
            direction.y * obb.axes[1] * obb.halfExtents.y +
            direction.z * obb.axes[2] * obb.halfExtents.z;
    }
    
    // Find min and max of all corners
    glm::vec3 aabbMin = corners[0];
    glm::vec3 aabbMax = corners[0];
    
    for (int i = 1; i < 8; i++) {
        aabbMin = glm::min(aabbMin, corners[i]);
        aabbMax = glm::max(aabbMax, corners[i]);
    }
    
    // Create an AABB for testing
    Aabb aabb(aabbMin, aabbMax);
    return TestAabbAgainstFrustum(aabb);
}

glm::vec3 CameraSystem::GetFrustumTestColor(SideResult result) const
{
    switch (result) {
        case SideResult::eINSIDE:
            return FRUSTUM_INSIDE_COLOR;
        case SideResult::eOUTSIDE:
            return FRUSTUM_OUTSIDE_COLOR;
        case SideResult::eOVERLAPPING:
            return FRUSTUM_INTERSECT_COLOR;
        default:
            return glm::vec3(1.0f); // White as fallback
    }
}

glm::mat4 CameraSystem::GetVisualizationViewProjectionMatrix(const CameraComponent& camera, float aspectRatio) const
{
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = m_ReferenceCameraProjection.GetProjectionMatrix(aspectRatio);
    
    return projection * view;
} 