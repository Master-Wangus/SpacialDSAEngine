/**
 * @class InputSystem
 * @brief System for handling user input from keyboard, mouse and other devices.
 *
 * This system manages input events, state tracking, and provides a framework for
 * binding input actions to game logic, supporting both polling and event-driven approaches.
 */

#include "pch.h"
#include "InputSystem.hpp"
#include "Window.hpp"
#include "Components.hpp"
#include "RayRenderer.hpp"
#include "Systems.hpp"
#include "Keybinds.hpp"
#include "EventSystem.hpp"

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
    // Cleanup
}

void InputSystem::Update(float deltaTime)
{
    // Currently no per-frame updates needed
}

bool InputSystem::IsKeyPressed(int keyCode) const
{
    return m_Window.IsKeyPressed(keyCode);
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



void InputSystem::StartDragging()
{
    // Only start dragging if ImGui isn't capturing the mouse
    if (!ImGui::GetIO().WantCaptureMouse) 
    {
        m_MouseDragging = true;
        m_Window.SetInputMode(Keybinds::CURSOR, Keybinds::CURSOR_DISABLED);
    }
}

void InputSystem::StopDragging()
{
    m_MouseDragging = false;
    m_Window.SetInputMode(Keybinds::CURSOR, Keybinds::CURSOR_NORMAL);
}

void InputSystem::ProcessKeyCallback(int key, int scancode, int action, int mods)
{
    // Skip keyboard input if ImGui is capturing keyboard
    if (ImGui::GetIO().WantCaptureKeyboard) 
    {
        return;
    }
    
    // Update key state
    if (action == Keybinds::PRESS) 
    {
        m_KeysPressed[key] = true;
        
        // Fire event using the EventSystem
        FIRE_EVENT_WITH_DATA(EventType::KeyPress, key);
    }
    else if (action == Keybinds::RELEASE) 
    {
        m_KeysPressed[key] = false;
        
        // Fire event using the EventSystem
        FIRE_EVENT_WITH_DATA(EventType::KeyRelease, key);
    }
    

}

void InputSystem::ProcessMouseButtonCallback(int button, int action, int mods)
{
    // Skip mouse input if ImGui is capturing mouse
    if (ImGui::GetIO().WantCaptureMouse) 
    {
        return;
    }
    
    // Update mouse button state
    if (action == Keybinds::PRESS) 
    {
        m_MouseButtonsPressed[button] = true;
        
        // Fire event using the EventSystem
        FIRE_EVENT_WITH_DATA(EventType::MouseButtonPress, button);
    } else if (action == Keybinds::RELEASE) 
    {
        m_MouseButtonsPressed[button] = false;
        
        // Fire event using the EventSystem
        FIRE_EVENT_WITH_DATA(EventType::MouseButtonRelease, button);
    }
    

}

void InputSystem::ProcessCursorPosCallback(double xpos, double ypos)
{
    // Update mouse position
    m_CurrentMousePos = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
    
    // Calculate delta only when dragging and not captured by ImGui
    if (m_MouseDragging && !ImGui::GetIO().WantCaptureMouse) 
    {
        m_MouseDelta.x = m_CurrentMousePos.x - m_LastMousePos.x;
        m_MouseDelta.y = m_LastMousePos.y - m_CurrentMousePos.y; // Inverted Y for camera control
    } else {
        m_MouseDelta = glm::vec2(0.0f);
    }
    
    m_LastMousePos = m_CurrentMousePos;
    
    // Fire event using the EventSystem if not captured by ImGui
    if (!ImGui::GetIO().WantCaptureMouse) {
        FIRE_EVENT_WITH_DATA(EventType::MouseMove, m_CurrentMousePos);
    }
    

}

void InputSystem::ProcessScrollCallback(double xoffset, double yoffset)
{
    // Skip scroll input if ImGui is capturing mouse
    if (ImGui::GetIO().WantCaptureMouse) 
    {
        return;
    }
    
    // Fire event using the EventSystem
    FIRE_EVENT_WITH_DATA(EventType::MouseScroll, glm::vec2(static_cast<float>(xoffset), static_cast<float>(yoffset)));
    

} 