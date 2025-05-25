#include "pch.h"
#include "FPSCameraSystem.hpp"
#include "Window.hpp"
#include "InputSystem.hpp"
#include "Systems.hpp"
#include "ObjectManipulationSystem.hpp"
#include "Keybinds.hpp"
#include "Registry.hpp"
#include "Components.hpp"

FPSCameraSystem::FPSCameraSystem(Registry& registry, Window& window)
    : m_Registry(registry), m_Window(window)
{
    auto cameraView = registry.View<CameraComponent>();
    if (!cameraView.empty()) {
        m_CameraEntity = *cameraView.begin();
    } 
    else 
    {
        
        m_CameraEntity = registry.Create();
        
        FPSCameraComponent fpsCamera(
            glm::vec3(0.0f, 0.0f, 3.0f),  // position
            glm::vec3(0.0f, 0.0f, -1.0f), // front
            glm::vec3(0.0f, 1.0f, 0.0f)   // up
        );
        
        ProjectionComponent projection(
            45.0f,  // fov
            0.1f,   // near plane
            100.0f  // far plane
        );
        
        CameraComponent camera;
        camera.m_FPS = fpsCamera;
        camera.m_Projection = projection;
        camera.m_ActiveCameraType = CameraType::FPS;
        
        registry.AddComponent<CameraComponent>(m_CameraEntity, camera);
    }
    
    // Set up input callbacks for this system
    SetupInputCallbacks();
}

void FPSCameraSystem::SetupInputCallbacks()
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
                    // Only start camera control if we're not dragging an object
                    if (!Systems::g_ObjectManipulationSystem->IsDragging())
                    {
                        Systems::g_InputSystem->StartDragging();
                    }
                }
                else if (action == Keybinds::RELEASE)
                {
                    Systems::g_InputSystem->StopDragging();
                }
            }
        });
}

void FPSCameraSystem::OnRun(float deltaTime)
{
    // Only process input if we have a valid camera entity
    auto cameraView = m_Registry.View<CameraComponent>();
    if (!cameraView.empty()) {
        ProcessKeyboardInput(deltaTime);
    }
}

glm::mat4 FPSCameraSystem::GetViewMatrix(const CameraComponent& camera)
{
    return camera.GetViewMatrix();
}

void FPSCameraSystem::ProcessMouseMovement()
{
    // Get mouse delta from input system
    glm::vec2 mouseDelta = Systems::g_InputSystem->GetMouseDelta();
    
    // Skip if no movement
    if (glm::length(mouseDelta) < 0.0001f)
        return;
    
    float xOffset = mouseDelta.x;
    float yOffset = mouseDelta.y;
    
    // Update camera
    auto& camera = m_Registry.GetComponent<CameraComponent>(m_CameraEntity);
    
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

void FPSCameraSystem::ProcessKeyboardInput(float deltaTime)
{
    auto& camera = m_Registry.GetComponent<CameraComponent>(m_CameraEntity);
    
    // Get keys state from input system
    bool keyW = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_W);
    bool keyS = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_S);
    bool keyA = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_A);
    bool keyD = Systems::g_InputSystem->IsKeyPressed(Keybinds::KEY_D);
    
    float cameraSpeed = camera.m_FPS.m_MovementSpeed * deltaTime;
    
    // Update camera position
    if (keyW)
        camera.m_FPS.m_CameraPosition += camera.m_FPS.m_CameraFront * cameraSpeed;
    if (keyS)
        camera.m_FPS.m_CameraPosition -= camera.m_FPS.m_CameraFront * cameraSpeed;
    if (keyA)
        camera.m_FPS.m_CameraPosition -= glm::normalize(glm::cross(camera.m_FPS.m_CameraFront, camera.m_FPS.m_CameraUpDirection)) * cameraSpeed;
    if (keyD)
        camera.m_FPS.m_CameraPosition += glm::normalize(glm::cross(camera.m_FPS.m_CameraFront, camera.m_FPS.m_CameraUpDirection)) * cameraSpeed;
} 