#pragma once

#include "Components.hpp"
#include "Registry.hpp"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Window;

class FPSCameraSystem {
public:
    FPSCameraSystem(Registry& registry, Window& window);
    void OnRun(float deltaTime);
    static glm::mat4 GetViewMatrix(const CameraComponent& camera);

private:
    void ProcessMouseMovement(double xpos, double ypos);
    void ProcessMouseButton(int button, int action, int mods);
    void ProcessKeyboardInput(float deltaTime);

    Registry& m_Registry;
    Window& m_Window;
    
    // Mouse tracking
    bool m_FirstMouse = true;
    float m_LastX = 0.0f;
    float m_LastY = 0.0f;
    bool m_MouseDragging = false;  // Is mouse button currently pressed?
    
    // Camera entity
    Registry::Entity m_CameraEntity;
}; 