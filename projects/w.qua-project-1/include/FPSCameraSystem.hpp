#pragma once

#include "pch.h"

// Forward declarations
class Registry;
class Window;
struct CameraComponent;

class FPSCameraSystem 
{
public:
    FPSCameraSystem(Registry& registry, Window& window);
    
    void OnRun(float deltaTime);
    
    glm::mat4 GetViewMatrix(const CameraComponent& camera);

private:
    void SetupInputCallbacks();
    void ProcessMouseMovement();
    void ProcessKeyboardInput(float deltaTime);
    
    Registry& m_Registry;
    Window& m_Window;
    Registry::Entity m_CameraEntity = entt::null;
}; 