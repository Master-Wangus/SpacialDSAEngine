#include "FPSCameraSystem.hpp"
#include "Window.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

FPSCameraSystem::FPSCameraSystem(Registry& registry, Window& window)
    : m_Registry(registry), m_Window(window)
{
    auto cameraView = registry.View<CameraComponent>();
    if (!cameraView.empty()) {
        m_CameraEntity = *cameraView.begin();
    } 
    else 
    {
        // Create a default camera
        std::cout << "FPSCameraSystem: Creating default camera" << std::endl;
        
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
    
    // Set initial mouse position for both cases
    m_LastX = static_cast<float>(window.GetWidth() / 2.0f);
    m_LastY = static_cast<float>(window.GetHeight() / 2.0f);
    
    window.SetCursorPosCallback([this](double xpos, double ypos) {
        this->ProcessMouseMovement(xpos, ypos);
    });
    
    window.SetMouseButtonCallback([this](int button, int action, int mods) {
        this->ProcessMouseButton(button, action, mods);
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

void FPSCameraSystem::ProcessMouseButton(int button, int action, int mods)
{
    // Set dragging flag based on left mouse button state
    if (button == Window::MOUSE_BUTTON_LEFT) {
        if (action == Window::PRESS) {
            m_MouseDragging = true;
            
            // Reset first mouse flag so we don't get a jump when starting to drag
            m_FirstMouse = true;
        } 
        else if (action == Window::RELEASE) {
            m_MouseDragging = false;
        }
    }
}

void FPSCameraSystem::ProcessMouseMovement(double xpos, double ypos)
{
    // Only process mouse movement if we're dragging
    if (!m_MouseDragging)
        return;
        
    if (m_FirstMouse) {
        m_LastX = static_cast<float>(xpos);
        m_LastY = static_cast<float>(ypos);
        m_FirstMouse = false;
        return; // Skip the first frame to avoid jumps
    }
    
    float xOffset = static_cast<float>(xpos) - m_LastX;
    float yOffset = m_LastY - static_cast<float>(ypos);
    
    m_LastX = static_cast<float>(xpos);
    m_LastY = static_cast<float>(ypos);
    
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
    
    // Get keys state
    bool keyW = m_Window.IsKeyPressed(Window::KEY_W);
    bool keyS = m_Window.IsKeyPressed(Window::KEY_S);
    bool keyA = m_Window.IsKeyPressed(Window::KEY_A);
    bool keyD = m_Window.IsKeyPressed(Window::KEY_D);
    
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