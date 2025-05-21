#include "FPSCameraSystem.hpp"
#include "Window.hpp"
#include "InputSystem.hpp"
#include "Systems.hpp"
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
    
    // Set up input callbacks for this system
    SetupInputCallbacks();
}

void FPSCameraSystem::SetupInputCallbacks()
{
    // Subscribe to mouse movement
    Systems::g_InputSystem->SubscribeToMouseMove([this](double xpos, double ypos) {
        if (Systems::g_InputSystem->IsMouseDragging()) 
        {
            this->ProcessMouseMovement(xpos, ypos);
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

void FPSCameraSystem::ProcessMouseMovement(double xpos, double ypos)
{
    static bool firstMouse = true;
    static float lastX = 0.0f;
    static float lastY = 0.0f;
    
    if (firstMouse) 
    {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
        return; // Skip the first frame to avoid jumps
    }
    
    // Get mouse delta from input system
    glm::vec2 mouseDelta = Systems::g_InputSystem->GetMouseDelta();
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
    bool keyW = Systems::g_InputSystem->IsKeyPressed(Window::KEY_W);
    bool keyS = Systems::g_InputSystem->IsKeyPressed(Window::KEY_S);
    bool keyA = Systems::g_InputSystem->IsKeyPressed(Window::KEY_A);
    bool keyD = Systems::g_InputSystem->IsKeyPressed(Window::KEY_D);
    
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