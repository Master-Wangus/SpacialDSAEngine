#pragma once

#include <functional>
#include <map>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Registry.hpp"

class Window;

// Input event types
enum class InputEventType {
    KeyPressed,
    KeyReleased,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseScrolled
};

// Callback types
using KeyCallback = std::function<void(int, int, int, int)>;
using MouseButtonCallback = std::function<void(int, int, int)>;
using MouseMoveCallback = std::function<void(double, double)>;
using MouseScrollCallback = std::function<void(double, double)>;

class InputSystem {
public:
    InputSystem(Registry& registry, Window& window);
    ~InputSystem();

    void Update(float deltaTime);

    // Input state checking
    bool IsKeyPressed(int keyCode) const;
    bool WasKeyPressed(int keyCode) const;
    bool IsMouseButtonPressed(int button) const;
    
    // Getting mouse state
    glm::vec2 GetMousePosition() const;
    glm::vec2 GetMouseDelta() const;
    bool IsMouseDragging() const;
    
    // Event subscription
    void SubscribeToKey(int keyCode, KeyCallback callback);
    void SubscribeToMouseButton(int button, MouseButtonCallback callback);
    void SubscribeToMouseMove(MouseMoveCallback callback);
    void SubscribeToMouseScroll(MouseScrollCallback callback);
    
    // Mouse dragging control
    void StartDragging();
    void StopDragging();

private:
    Registry& m_Registry;
    Window& m_Window;
    
    // Mouse state
    bool m_MouseDragging;
    glm::vec2 m_LastMousePos;
    glm::vec2 m_CurrentMousePos;
    glm::vec2 m_MouseDelta;
    
    // Keyboard state
    std::map<int, bool> m_KeysPressed;
    std::map<int, bool> m_PreviousKeysPressed;
    
    // Mouse button state
    std::map<int, bool> m_MouseButtonsPressed;
    
    // Callback handlers
    void ProcessKeyCallback(int key, int scancode, int action, int mods);
    void ProcessMouseButtonCallback(int button, int action, int mods);
    void ProcessCursorPosCallback(double xpos, double ypos);
    void ProcessScrollCallback(double xoffset, double yoffset);
    
    // Callback storage
    std::multimap<int, KeyCallback> m_KeyCallbacks;
    std::multimap<int, MouseButtonCallback> m_MouseButtonCallbacks;
    std::vector<MouseMoveCallback> m_MouseMoveCallbacks;
    std::vector<MouseScrollCallback> m_MouseScrollCallbacks;
}; 