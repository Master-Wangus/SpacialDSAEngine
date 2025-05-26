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
    m_PreviousKeysPressed = m_KeysPressed;
    HandleRayRotation(deltaTime);
}

// Function to handle ray rotation with Q and E keys
void InputSystem::HandleRayRotation(float deltaTime)
{
    const float rotationSpeed = 2.0f; // Radians per second
    bool qPressed = IsKeyPressed(Keybinds::KEY_Q);
    bool ePressed = IsKeyPressed(Keybinds::KEY_E);
    
    if (!qPressed && !ePressed) 
    {
        return; // No rotation keys pressed
    }
    
    // Find ray entities in the registry
    auto view = m_Registry.View<CollisionComponent, RenderComponent>();
    for (auto entity : view)
    {
        auto& collisionComp = m_Registry.GetComponent<CollisionComponent>(entity);
        
        if (collisionComp.m_ShapeType != CollisionShapeType::Ray) {
            continue;
        }
        
        auto& renderComp = m_Registry.GetComponent<RenderComponent>(entity);
        auto rayRenderer = std::dynamic_pointer_cast<RayRenderer>(renderComp.m_Renderable);
        
        if (!rayRenderer) {
            continue;
        }
        
        glm::vec3 currentDir = rayRenderer->GetDirection();
        
        // Calculate rotation axis (Y-axis for rotation in XZ plane)
        glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
        
        // Calculate rotation angle based on which key is pressed
        float rotationAngle = 0.0f;
        if (qPressed) 
        {
            rotationAngle = rotationSpeed * deltaTime; // Counterclockwise rotation
        }
        if (ePressed) 
        {
            rotationAngle = -rotationSpeed * deltaTime; // Clockwise rotation
        }
        
        // Create rotation quaternion
        glm::quat rotation = glm::angleAxis(rotationAngle, rotationAxis);
        
        // Apply rotation to direction
        glm::vec3 newDirection = rotation * currentDir;
        
        // Update ray renderer and collision component
        rayRenderer->SetDirection(newDirection);
        collisionComp.m_Ray.m_Direction = newDirection;
    }
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
    }
    else if (action == Keybinds::RELEASE) 
    {
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
    if (ImGui::GetIO().WantCaptureMouse) 
    {
        return;
    }
    
    // Update mouse button state
    if (action == Keybinds::PRESS) 
    {
        m_MouseButtonsPressed[button] = true;
    } else if (action == Keybinds::RELEASE) 
    {
        m_MouseButtonsPressed[button] = false;
    }
    
    // Notify subscribers first (object manipulation system will handle object selection)
    auto range = m_MouseButtonCallbacks.equal_range(button);
    for (auto it = range.first; it != range.second; ++it) 
    {
        it->second(button, action, mods);
    }
    
    // If no subscribers handled it, and it's left mouse button, handle camera control
    // This is now handled by the FPSCameraSystem
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
    if (ImGui::GetIO().WantCaptureMouse) 
    {
        return;
    }
    
    // Notify subscribers
    for (const auto& callback : m_MouseScrollCallbacks) {
        callback(xoffset, yoffset);
    }
} 