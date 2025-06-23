/**
 * @class Window
 * @brief OpenGL rendering window and application context.
 *
 * This class manages the creation and lifecycle of an OpenGL rendering window,
 * including initialization, event handling, and the main application loop.
 */

#pragma once

#include "pch.h"

// Forward declarations
struct GLFWwindow;
class InputSystem;

class Window 
{
public:
    // Callback types
    using FramebufferSizeCallback = std::function<void(int width, int height)>;
    using KeyCallback = std::function<void(int key, int scancode, int action, int mods)>;
    using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
    using CursorPosCallback = std::function<void(double xpos, double ypos)>;
    using ScrollCallback = std::function<void(double xoffset, double yoffset)>;
    

    
    /**
     * @brief Constructs a new window with specified dimensions and title.
     * @param width Window width in pixels
     * @param height Window height in pixels
     * @param title Window title string
     */
    Window(int width, int height, const std::string& title);
    
    /**
     * @brief Destructor that cleans up the window resources.
     */
    ~Window();
    
    // Window state
    /**
     * @brief Gets the underlying GLFW window handle.
     * @return Pointer to the GLFW window handle
     */
    GLFWwindow* GetHandle() const;
    
    /**
     * @brief Gets the native window handle.
     * @return Pointer to the native window handle
     */
    void* GetNativeWindow() const;
    
    /**
     * @brief Checks if the window should close.
     * @return True if the window should close, false otherwise
     */
    bool ShouldClose() const;
    
    /**
     * @brief Sets whether the window should close.
     * @param value True to mark window for closing, false otherwise
     */
    void SetShouldClose(bool value) const;
    
    /**
     * @brief Makes this window's OpenGL context current.
     */
    void MakeContextCurrent() const;
    
    /**
     * @brief Gets the current window width.
     * @return Window width in pixels
     */
    int GetWidth() const;
    
    /**
     * @brief Gets the current window height.
     * @return Window height in pixels
     */
    int GetHeight() const;
    
    /**
     * @brief Checks if a key is currently pressed.
     * @param key Key code to check
     * @return True if the key is pressed, false otherwise
     */
    bool IsKeyPressed(int key) const;
    
    /**
     * @brief Gets the current time in seconds.
     * @return Current time since GLFW initialization
     */
    double GetTime() const;

    /**
     * @brief Sets the keyboard callback function.
     * @param callback Function to call on keyboard events
     */
    void SetKeyCallback(std::function<void(int, int, int, int)> callback);
    
    /**
     * @brief Sets the cursor position callback function.
     * @param callback Function to call on cursor movement
     */
    void SetCursorPosCallback(std::function<void(double, double)> callback);
    
    /**
     * @brief Sets the mouse button callback function.
     * @param callback Function to call on mouse button events
     */
    void SetMouseButtonCallback(std::function<void(int, int, int)> callback);
    
    /**
     * @brief Sets the scroll callback function.
     * @param callback Function to call on scroll events
     */
    void SetScrollCallback(std::function<void(double, double)> callback);
    
    /**
     * @brief Sets the framebuffer size callback function.
     * @param callback Function to call when framebuffer size changes
     */
    void SetFramebufferSizeCallback(std::function<void(int, int)> callback);
    
    // Window operation
    /**
     * @brief Processes all pending window events.
     */
    void PollEvents() const;
    
    /**
     * @brief Swaps the front and back buffers.
     */
    void SwapBuffers() const;
    
    /**
     * @brief Sets the window title.
     * @param title New window title string
     */
    void SetTitle(const std::string& title);
    
    // Input handling
    /**
     * @brief Checks if a mouse button is currently pressed.
     * @param button Mouse button code to check
     * @return True if the button is pressed, false otherwise
     */
    bool IsMouseButtonPressed(int button) const;
    
    /**
     * @brief Gets the current cursor position.
     * @param xpos Pointer to store x coordinate
     * @param ypos Pointer to store y coordinate
     */
    void GetCursorPos(double* xpos, double* ypos) const;
    
    /**
     * @brief Sets an input mode value.
     * @param mode Input mode to set
     * @param value Value to set for the mode
     */
    void SetInputMode(int mode, int value) const;
        
    // Make InputSystem a friend so it can access our private members
    friend class InputSystem;

private:
    GLFWwindow* m_Window;         ///< The GLFW window handle
    int m_Width;                 
    int m_Height;                 
    std::string m_Title;          

    // Callback storage
    FramebufferSizeCallback m_FramebufferSizeCallback;
    KeyCallback m_KeyCallback;
    MouseButtonCallback m_MouseButtonCallback;
    CursorPosCallback m_CursorPosCallback;
    ScrollCallback m_ScrollCallback;
    
    // Static callback wrappers for GLFW (needed to map to member functions)
    static void KeyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void CursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos);
    static void MouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods);
    static void ScrollCallbackWrapper(GLFWwindow* window, double xoffset, double yoffset);
    static void FramebufferSizeCallbackWrapper(GLFWwindow* window, int width, int height);
}; 