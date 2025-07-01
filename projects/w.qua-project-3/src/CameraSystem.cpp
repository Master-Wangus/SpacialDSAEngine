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
#include "EventSystem.hpp"
#include "Systems.hpp"
#include "Keybinds.hpp"
#include "Registry.hpp"
#include "Components.hpp"
#include "Geometry.hpp" // Added for frustum culling functions

CameraSystem::CameraSystem(Registry& registry, Window& window)
    : m_Registry(registry), m_Window(window)
{
    auto cameraView = registry.View<CameraComponent>();

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

    if (!cameraView.empty()) 
    {
        m_CameraEntity = *cameraView.begin();
    } 
    else 
    {
        
        m_CameraEntity = registry.Create();
        
        CameraComponent camera;
        camera.m_FPS = fpsCamera;
        camera.m_TopDown = orbitalCamera;
        camera.m_Projection = projection;
        camera.m_ActiveCameraType = CameraType::FPS;
        
        registry.AddComponent<CameraComponent>(m_CameraEntity, camera);
    }
    
    // Initialize frustum planes
    for (int i = 0; i < 6; ++i) 
    {
        m_FrustumNormals[i] = glm::vec3(0.0f);
        m_FrustumDistances[i] = 0.0f;
    }
    
    auto& camera = registry.GetComponent<CameraComponent>(m_CameraEntity);
    m_ReferenceCameraProjection = projection;
    
    // Set up event subscriptions
    SetupEventSubscriptions();
}

CameraSystem::~CameraSystem()
{
    // No cleanup needed
}

void CameraSystem::SetupEventSubscriptions()
{
    // Subscribe to mouse movement events
    EventSystem::Get().SubscribeToEvent(EventType::MouseMove, [this](const EventData& eventData) {
        if (m_MouseDragging)
        {
            this->ProcessMouseMovement(eventData);
        }
    });
    
    // Subscribe to mouse button press events
    EventSystem::Get().SubscribeToEvent(EventType::MouseButtonPress, [this](const EventData& eventData) {
        this->HandleMouseButtonPressEvent(eventData);
    });
    
    // Subscribe to mouse button release events
    EventSystem::Get().SubscribeToEvent(EventType::MouseButtonRelease, [this](const EventData& eventData) {
        this->HandleMouseButtonReleaseEvent(eventData);
    });
    
    // Subscribe to mouse scroll events
    EventSystem::Get().SubscribeToEvent(EventType::MouseScroll, [this](const EventData& eventData) {
        this->HandleMouseScrollEvent(eventData);
    });
    
    // Subscribe to key press events
    EventSystem::Get().SubscribeToEvent(EventType::KeyPress, [this](const EventData& eventData) {
        this->HandleKeyPressEvent(eventData);
    });
    
    // Subscribe to key release events
    EventSystem::Get().SubscribeToEvent(EventType::KeyRelease, [this](const EventData& eventData) {
        this->HandleKeyReleaseEvent(eventData);
    });
}

void CameraSystem::HandleMouseButtonPressEvent(const EventData& eventData)
{
    if (auto buttonCode = std::get_if<int>(&eventData))
    {
        if (*buttonCode == Keybinds::MOUSE_BUTTON_RIGHT) 
        {
            m_MouseDragging = true;
            
            // Reset last mouse position to current position to prevent jumping
            m_LastMousePos = Systems::GetInputSystem()->GetMousePosition();
        }
    }
}

void CameraSystem::HandleMouseButtonReleaseEvent(const EventData& eventData)
{
    if (auto buttonCode = std::get_if<int>(&eventData))
    {
        if (*buttonCode == Keybinds::MOUSE_BUTTON_RIGHT) 
        {
            m_MouseDragging = false;
        }
    }
}

void CameraSystem::HandleMouseScrollEvent(const EventData& eventData)
{
    if (auto scrollData = std::get_if<glm::vec2>(&eventData)) 
    {
        auto& camera = m_Registry.GetComponent<CameraComponent>(m_CameraEntity);
        if (camera.m_ActiveCameraType == CameraType::Orbital) 
        {
            camera.m_TopDown.Zoom(-scrollData->y);
        }
    }
}

void CameraSystem::HandleKeyPressEvent(const EventData& eventData)
{
    if (auto keyCode = std::get_if<int>(&eventData)) 
    {
        if (*keyCode == Keybinds::KEY_C) 
        {
            if (!m_CKeyPressed)
            {
                SwitchCameraType();
                m_CKeyPressed = true;
            }
        }
    }
}

void CameraSystem::HandleKeyReleaseEvent(const EventData& eventData)
{
    if (auto keyCode = std::get_if<int>(&eventData)) 
    {
        if (*keyCode == Keybinds::KEY_C) 
        {
            m_CKeyPressed = false;
        }
    }
}

void CameraSystem::OnRun(float deltaTime)
{
    auto cameraView = m_Registry.View<CameraComponent>();
    if (!cameraView.empty()) 
    {
        ProcessKeyboardInput(deltaTime);
    }
}

glm::mat4 CameraSystem::GetViewMatrix(const CameraComponent& camera)
{
    return camera.GetViewMatrix();
}

void CameraSystem::ProcessMouseMovement(const EventData& eventData)
{
    if (auto mousePos = std::get_if<glm::vec2>(&eventData)) {
        glm::vec2 currentPos = *mousePos;
        glm::vec2 delta = currentPos - m_LastMousePos;
        m_LastMousePos = currentPos;
        
        // Skip if no movement
        if (glm::length(delta) < 0.0001f)
            return;
        
        float xOffset = delta.x;
        float yOffset = delta.y;
        
        auto& camera = m_Registry.GetComponent<CameraComponent>(m_CameraEntity);
        
        if (camera.m_ActiveCameraType == CameraType::FPS) 
        {
            const float sensitivity = 0.1f;
            xOffset *= sensitivity;
            yOffset *= sensitivity;
            
            camera.m_FPS.m_YawAngle += xOffset;
            camera.m_FPS.m_PitchAngle -= yOffset;  // Inverted 
            
            // Constrain pitch
            if (camera.m_FPS.m_PitchAngle > 89.0f) camera.m_FPS.m_PitchAngle = 89.0f;
            if (camera.m_FPS.m_PitchAngle < -89.0f) camera.m_FPS.m_PitchAngle = -89.0f;
            
            camera.m_FPS.UpdateVectors();
        }
        else if (camera.m_ActiveCameraType == CameraType::Orbital)
        {
            const float panSpeed = 0.05f;
            glm::vec3 offset(-xOffset * panSpeed, 0.0f, yOffset * panSpeed);
            camera.m_TopDown.MoveTarget(offset);
        }
    }
}

void CameraSystem::ProcessKeyboardInput(float deltaTime)
{
    auto& camera = m_Registry.GetComponent<CameraComponent>(m_CameraEntity);
    
    if (camera.m_ActiveCameraType == CameraType::FPS) 
    {
        bool keyW = Systems::GetInputSystem()->IsKeyPressed(Keybinds::KEY_W);
        bool keyS = Systems::GetInputSystem()->IsKeyPressed(Keybinds::KEY_S);
        bool keyA = Systems::GetInputSystem()->IsKeyPressed(Keybinds::KEY_A);
        bool keyD = Systems::GetInputSystem()->IsKeyPressed(Keybinds::KEY_D);
        
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
        bool keyW = Systems::GetInputSystem()->IsKeyPressed(Keybinds::KEY_W);
        bool keyS = Systems::GetInputSystem()->IsKeyPressed(Keybinds::KEY_S);
        bool keyA = Systems::GetInputSystem()->IsKeyPressed(Keybinds::KEY_A);
        bool keyD = Systems::GetInputSystem()->IsKeyPressed(Keybinds::KEY_D);
        
        // Move target point in orbital mode
        float targetSpeed = 2.0f * deltaTime;
        
        if (keyW)
            camera.m_TopDown.MoveTarget(glm::vec3(0.0f, 0.0f, -targetSpeed)); // Move forward
        if (keyS)
            camera.m_TopDown.MoveTarget(glm::vec3(0.0f, 0.0f, targetSpeed)); // Move backward
        if (keyA)
            camera.m_TopDown.MoveTarget(glm::vec3(-targetSpeed, 0.0f, 0.0f)); // Move left
        if (keyD)
            camera.m_TopDown.MoveTarget(glm::vec3(targetSpeed, 0.0f, 0.0f)); // Move right
    }
}

void CameraSystem::SwitchCameraType()
{
    auto& camera = m_Registry.GetComponent<CameraComponent>(m_CameraEntity);
    
    if (camera.m_ActiveCameraType == CameraType::FPS)
    {
        // Switching to Top-Down
        // Store current FPS state so we can restore later.
        m_StoredFPSPosition = camera.m_FPS.m_CameraPosition;
        m_StoredFPSYaw      = camera.m_FPS.m_YawAngle;
        m_StoredFPSPitch    = camera.m_FPS.m_PitchAngle;
        m_HasStoredFPSState = true;

        // If we previously had a stored top-down state, restore it; else init from FPS position.
        if (m_HasStoredTDState)
        {
            camera.m_TopDown.m_Target   = m_StoredTDTarget;
            camera.m_TopDown.m_Distance = m_StoredTDDistance;
        }
        else
        {
            // Compute scene centre from all entities' world-space AABBs so the
            // first top-down view starts centred on the content.
            glm::vec3 minWorld(std::numeric_limits<float>::max());
            glm::vec3 maxWorld(-std::numeric_limits<float>::max());
            bool hasObjects = false;

            auto aabbView = m_Registry.View<BoundingComponent>();
            for (auto ent : aabbView)
            {
                auto& bc = m_Registry.GetComponent<BoundingComponent>(ent);
                Aabb worldAabb = bc.GetAABB();

                // Bring to world space if the entity has a transform.
                if (m_Registry.HasComponent<TransformComponent>(ent))
                {
                    const auto& model = m_Registry.GetComponent<TransformComponent>(ent).m_Model;
                    worldAabb.Transform(model);
                }

                minWorld = glm::min(minWorld, worldAabb.min);
                maxWorld = glm::max(maxWorld, worldAabb.max);
                hasObjects = true;
            }

            if (hasObjects)
            {
                glm::vec3 sceneCentre = 0.5f * (minWorld + maxWorld);
                camera.m_TopDown.m_Target = sceneCentre;

                // Choose a distance that frames the entire scene (simple heuristic).
                float sceneRadius = 0.5f * glm::length(maxWorld - minWorld);
                camera.m_TopDown.m_Distance = glm::clamp(sceneRadius * 2.0f, 5.0f, 100.0f);
            }
            else
            {
                // Fallback: use FPS position as before.
                camera.m_TopDown.m_Target   = camera.m_FPS.m_CameraPosition;
                camera.m_TopDown.m_Distance = 10.0f;
            }
        }

        camera.m_ActiveCameraType = CameraType::Orbital;
    }
    else
    {
        // Switching back to FPS
        // Save current top-down state for later restoration
        m_StoredTDTarget   = camera.m_TopDown.m_Target;
        m_StoredTDDistance = camera.m_TopDown.m_Distance;
        m_HasStoredTDState = true;

        camera.m_ActiveCameraType = CameraType::FPS;

        if (m_HasStoredFPSState)
        {
            camera.m_FPS.m_CameraPosition = m_StoredFPSPosition;
            camera.m_FPS.m_YawAngle       = m_StoredFPSYaw;
            camera.m_FPS.m_PitchAngle     = m_StoredFPSPitch;
            camera.m_FPS.UpdateVectors();
        }
        else
        {
            // Fallback to position at the top-down camera location facing its target.
            camera.m_FPS.m_CameraPosition = camera.m_TopDown.GetCameraPosition();
            glm::vec3 direction = camera.m_TopDown.m_Target - camera.m_FPS.m_CameraPosition;
            if (glm::length(direction) > 0.001f)
            {
                direction = glm::normalize(direction);
                camera.m_FPS.m_CameraFront = direction;
                camera.m_FPS.m_YawAngle   = glm::degrees(atan2(direction.z, direction.x));
                camera.m_FPS.m_PitchAngle = glm::degrees(asin(direction.y));
            }
        }
    }
}

void CameraSystem::UpdateFrustumPlanes(const CameraComponent& camera, float aspectRatio)
{
    // Calculate view-projection matrix using the reference camera projection (not the main camera's projection)
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = m_ReferenceCameraProjection.GetProjectionMatrix(aspectRatio);
    glm::mat4 viewProjection = projection * view;
    
    FrustumFromVp(viewProjection, m_FrustumNormals, m_FrustumDistances);
    
    m_FrustumUpdated = true;
}

SideResult CameraSystem::TestSphereAgainstFrustum(const Sphere& sphere) const
{
    if (!m_FrustumUpdated) 
    {
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
    if (!m_FrustumUpdated) 
    {
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