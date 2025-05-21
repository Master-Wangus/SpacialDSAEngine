#include "pch.h"
#include "InputSystem.hpp"
#include "Window.hpp"

InputSystem::InputSystem(Registry& registry, Window& window)
    : m_Registry(registry), 
      m_Window(window), 
      m_MouseDragging(false),
      m_LastMousePos(0.0f, 0.0f),
      m_CurrentMousePos(0.0f, 0.0f),
      m_MouseDelta(0.0f, 0.0f)
{
    // Initialize mouse position to center of window
    float centerX = static_cast<float>(window.GetWidth()) / 2.0f;
    float centerY = static_cast<float>(window.GetHeight()) / 2.0f;
    m_LastMousePos = m_CurrentMousePos = glm::vec2(centerX, centerY);
    
    // Set callback functions
    window.SetKeyCallback([this](int key, int scancode, int action, int mods) {
        this->ProcessKeyCallback(key, scancode, action, mods);
    });
    
    window.SetMouseButtonCallback([this](int button, int action, int mods) {
        this->ProcessMouseButtonCallback(button, action, mods);
    });
    
    window.SetCursorPosCallback([this](double xpos, double ypos) {
        this->ProcessCursorPosCallback(xpos, ypos);
    });
    
    window.SetScrollCallback([this](double xoffset, double yoffset) {
        this->ProcessScrollCallback(xoffset, yoffset);
    });
}

InputSystem::~InputSystem()
{
    // Clear any stored callbacks
    m_KeyCallbacks.clear();
    m_MouseButtonCallbacks.clear();
    m_MouseMoveCallbacks.clear();
    m_MouseScrollCallbacks.clear();
}

void InputSystem::Update(float deltaTime)
{
    // Save current key states for the next frame
    m_PreviousKeysPressed = m_KeysPressed;
}

bool InputSystem::IsKeyPressed(int keyCode) const
{
    return m_Window.IsKeyPressed(keyCode);
}

bool InputSystem::WasKeyPressed(int keyCode) const
{
    auto it = m_PreviousKeysPressed.find(keyCode);
    return (it != m_PreviousKeysPressed.end() && it->second);
}

bool InputSystem::IsMouseButtonPressed(int button) const
{
    auto it = m_MouseButtonsPressed.find(button);
    return (it != m_MouseButtonsPressed.end() && it->second);
}

glm::vec2 InputSystem::GetMousePosition() const
{
    return m_CurrentMousePos;
}

glm::vec2 InputSystem::GetMouseDelta() const
{
    return m_MouseDelta;
}

bool InputSystem::IsMouseDragging() const
{
    return m_MouseDragging;
}

void InputSystem::SubscribeToKey(int keyCode, KeyCallback callback)
{
    m_KeyCallbacks.insert(std::make_pair(keyCode, callback));
}

void InputSystem::SubscribeToMouseButton(int button, MouseButtonCallback callback)
{
    m_MouseButtonCallbacks.insert(std::make_pair(button, callback));
}

void InputSystem::SubscribeToMouseMove(MouseMoveCallback callback)
{
    m_MouseMoveCallbacks.push_back(callback);
}

void InputSystem::SubscribeToMouseScroll(MouseScrollCallback callback)
{
    m_MouseScrollCallbacks.push_back(callback);
}

void InputSystem::StartDragging()
{
    // Only start dragging if ImGui isn't capturing the mouse
    if (!ImGui::GetIO().WantCaptureMouse) {
        m_MouseDragging = true;
        m_Window.SetInputMode(Window::CURSOR, Window::CURSOR_DISABLED);
    }
}

void InputSystem::StopDragging()
{
    m_MouseDragging = false;
    m_Window.SetInputMode(Window::CURSOR, Window::CURSOR_NORMAL);
}

void InputSystem::ProcessKeyCallback(int key, int scancode, int action, int mods)
{
    // Skip keyboard input if ImGui is capturing keyboard
    if (ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }
    
    // Update key state
    if (action == Window::PRESS) {
        m_KeysPressed[key] = true;
    } else if (action == Window::RELEASE) {
        m_KeysPressed[key] = false;
    }
    
    // Notify subscribers
    auto range = m_KeyCallbacks.equal_range(key);
    for (auto it = range.first; it != range.second; ++it) {
        it->second(key, scancode, action, mods);
    }
}

void InputSystem::ProcessMouseButtonCallback(int button, int action, int mods)
{
    // Skip mouse input if ImGui is capturing mouse
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    
    // Update mouse button state
    if (action == Window::PRESS) {
        m_MouseButtonsPressed[button] = true;
        
        // Left mouse button handling for camera control
        if (button == Window::MOUSE_BUTTON_LEFT) {
            StartDragging();
        }
    } else if (action == Window::RELEASE) {
        m_MouseButtonsPressed[button] = false;
        
        // Left mouse button handling for camera control
        if (button == Window::MOUSE_BUTTON_LEFT) {
            StopDragging();
        }
    }
    
    // Notify subscribers
    auto range = m_MouseButtonCallbacks.equal_range(button);
    for (auto it = range.first; it != range.second; ++it) {
        it->second(button, action, mods);
    }
}

void InputSystem::ProcessCursorPosCallback(double xpos, double ypos)
{
    // Update mouse position
    m_CurrentMousePos = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
    
    // Calculate delta only when dragging and not captured by ImGui
    if (m_MouseDragging && !ImGui::GetIO().WantCaptureMouse) {
        m_MouseDelta.x = m_CurrentMousePos.x - m_LastMousePos.x;
        m_MouseDelta.y = m_LastMousePos.y - m_CurrentMousePos.y; // Inverted Y for camera control
    }
    
    m_LastMousePos = m_CurrentMousePos;
    
    // Notify subscribers if not captured by ImGui
    if (!ImGui::GetIO().WantCaptureMouse) {
        for (const auto& callback : m_MouseMoveCallbacks) {
            callback(xpos, ypos);
        }
    }
}

void InputSystem::ProcessScrollCallback(double xoffset, double yoffset)
{
    // Skip scroll input if ImGui is capturing mouse
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    
    // Notify subscribers
    for (const auto& callback : m_MouseScrollCallbacks) {
        callback(xoffset, yoffset);
    }
} 