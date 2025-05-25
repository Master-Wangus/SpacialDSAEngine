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
    
    // Input mode constants
    static const int CURSOR;
    static const int CURSOR_NORMAL;
    static const int CURSOR_HIDDEN;
    static const int CURSOR_DISABLED;
    
    // Mouse button constants
    static const int MOUSE_BUTTON_LEFT;
    static const int MOUSE_BUTTON_RIGHT;
    static const int MOUSE_BUTTON_MIDDLE;
    
    // Action constants
    static const int PRESS;
    static const int RELEASE;
    
    Window(int width, int height, const std::string& title);
    ~Window();
    
    // Window state
    GLFWwindow* GetHandle() const;
    void* GetNativeWindow() const;
    bool ShouldClose() const;
    void SetShouldClose(bool value) const;
    void MakeContextCurrent() const;
    int GetWidth() const;
    int GetHeight() const;
    bool IsKeyPressed(int key) const;
    double GetTime() const;

    void SetKeyCallback(std::function<void(int, int, int, int)> callback);
    void SetCursorPosCallback(std::function<void(double, double)> callback);
    void SetMouseButtonCallback(std::function<void(int, int, int)> callback);
    void SetScrollCallback(std::function<void(double, double)> callback);
    void SetFramebufferSizeCallback(std::function<void(int, int)> callback);
    
    // Window operation
    void PollEvents() const;
    void SwapBuffers() const;
    void SetTitle(const std::string& title);
    
    // Input handling
    bool IsMouseButtonPressed(int button) const;
    void GetCursorPos(double* xpos, double* ypos) const;
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