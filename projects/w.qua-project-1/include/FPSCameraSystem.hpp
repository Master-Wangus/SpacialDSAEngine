#pragma once

#include "pch.h"
#include "Components.hpp"
#include "Registry.hpp"
#include <glm/glm.hpp>

class Window;

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
    Registry::Entity m_CameraEntity;
}; 