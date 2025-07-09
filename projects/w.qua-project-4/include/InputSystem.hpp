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
    

}; 