/**
 * @class InputSystem
 * @brief System for handling user input from keyboard, mouse and other devices.
 *
 * This system manages input events, state tracking, and provides a framework for
 * binding input actions to game logic, supporting both polling and event-driven approaches.
 */

#pragma once

#include "pch.h"

// Forward declarations
class Registry;
class Window;

// Input event types
enum class InputEventType 
{
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

class InputSystem 
{
public:
    /**
     * @brief Constructs the input system with registry and window references.
     * @param registry Entity registry for input-related components
     * @param window Window reference for input event handling
     */
    InputSystem(Registry& registry, Window& window);
    
    /**
     * @brief Destructor for the input system.
     */
    ~InputSystem();

    /**
     * @brief Updates the input system each frame.
     * @param deltaTime Time elapsed since last frame
     */
    void Update(float deltaTime);

    // Input state checking
    /**
     * @brief Checks if a key is currently pressed.
     * @param keyCode Key code to check
     * @return True if the key is currently pressed, false otherwise
     */
    bool IsKeyPressed(int keyCode) const;
    
    /**
     * @brief Checks if a key was pressed this frame.
     * @param keyCode Key code to check
     * @return True if the key was pressed this frame, false otherwise
     */
    bool WasKeyPressed(int keyCode) const;
    
    /**
     * @brief Checks if a mouse button is currently pressed.
     * @param button Mouse button code to check
     * @return True if the button is currently pressed, false otherwise
     */
    bool IsMouseButtonPressed(int button) const;
    
    // Getting mouse state
    /**
     * @brief Gets the current mouse position.
     * @return Current mouse position as a 2D vector
     */
    glm::vec2 GetMousePosition() const;
    
    /**
     * @brief Gets the mouse movement delta since last frame.
     * @return Mouse movement delta as a 2D vector
     */
    glm::vec2 GetMouseDelta() const;
    
    /**
     * @brief Checks if the mouse is currently being dragged.
     * @return True if mouse is being dragged, false otherwise
     */
    bool IsMouseDragging() const;
    
    // Event subscription
    /**
     * @brief Subscribes to key events for a specific key.
     * @param keyCode Key code to listen for
     * @param callback Function to call when key event occurs
     */
    void SubscribeToKey(int keyCode, KeyCallback callback);
    
    /**
     * @brief Subscribes to mouse button events for a specific button.
     * @param button Mouse button code to listen for
     * @param callback Function to call when mouse button event occurs
     */
    void SubscribeToMouseButton(int button, MouseButtonCallback callback);
    
    /**
     * @brief Subscribes to mouse movement events.
     * @param callback Function to call when mouse moves
     */
    void SubscribeToMouseMove(MouseMoveCallback callback);
    
    /**
     * @brief Subscribes to mouse scroll events.
     * @param callback Function to call when mouse scrolls
     */
    void SubscribeToMouseScroll(MouseScrollCallback callback);
    
    // Mouse dragging control
    /**
     * @brief Starts mouse dragging mode.
     */
    void StartDragging();
    
    /**
     * @brief Stops mouse dragging mode.
     */
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
    /**
     * @brief Processes keyboard callback events.
     * @param key Key code
     * @param scancode System-specific scancode
     * @param action Action type (press, release, repeat)
     * @param mods Modifier key flags
     */
    void ProcessKeyCallback(int key, int scancode, int action, int mods);
    
    /**
     * @brief Processes mouse button callback events.
     * @param button Mouse button code
     * @param action Action type (press, release)
     * @param mods Modifier key flags
     */
    void ProcessMouseButtonCallback(int button, int action, int mods);
    
    /**
     * @brief Processes cursor position callback events.
     * @param xpos X coordinate of cursor
     * @param ypos Y coordinate of cursor
     */
    void ProcessCursorPosCallback(double xpos, double ypos);
    
    /**
     * @brief Processes scroll callback events.
     * @param xoffset Horizontal scroll offset
     * @param yoffset Vertical scroll offset
     */
    void ProcessScrollCallback(double xoffset, double yoffset);
    
    // Callback storage
    std::multimap<int, KeyCallback> m_KeyCallbacks;
    std::multimap<int, MouseButtonCallback> m_MouseButtonCallbacks;
    std::vector<MouseMoveCallback> m_MouseMoveCallbacks;
    std::vector<MouseScrollCallback> m_MouseScrollCallbacks;
}; 