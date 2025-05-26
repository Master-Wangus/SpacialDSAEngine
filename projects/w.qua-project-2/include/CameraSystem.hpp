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