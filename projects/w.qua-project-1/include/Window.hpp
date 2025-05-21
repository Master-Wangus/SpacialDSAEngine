#pragma once

#include "pch.h"

#include <GLFW/glfw3.h> // As per task 1 requirements

// Forward declarations
struct GLFWwindow;
class InputSystem;

// Forward declaration for InputSystem friendship
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
    
    // Input modes
    static const int CURSOR = 0x00033001;
    static const int CURSOR_NORMAL = 0x00034001;
    static const int CURSOR_HIDDEN = 0x00034002;
    static const int CURSOR_DISABLED = 0x00034003;
    
    // Mouse buttons
    static const int MOUSE_BUTTON_LEFT = 0;
    static const int MOUSE_BUTTON_RIGHT = 1;
    static const int MOUSE_BUTTON_MIDDLE = 2;
    
    // Key actions
    static const int PRESS = 1;
    static const int RELEASE = 0;
    
    // Common keys
    static const int KEY_W = 87;
    static const int KEY_A = 65;
    static const int KEY_S = 83;
    static const int KEY_D = 68;
    static const int KEY_ESCAPE = 256;
    static const int KEY_SPACE = 32;
    static const int KEY_LEFT_SHIFT = 340;
    static const int KEY_RIGHT_SHIFT = 344;
    static const int KEY_LEFT_CONTROL = 341;
    static const int KEY_RIGHT_CONTROL = 345;

    Window(int width, int height, const std::string& title);
    ~Window();
    
    // Window state
    GLFWwindow* GetHandle() const;
    bool ShouldClose() const;
    void SetShouldClose(bool value) const;
    void MakeContextCurrent() const;
    int GetWidth() const;
    int GetHeight() const;
    bool IsKeyPressed(int key) const;
    double GetTime() const;
    
    // Window operation
    void PollEvents() const;
    void SwapBuffers() const;
    void SetTitle(const std::string& title);
    
    // Input handling
    bool IsMouseButtonPressed(int button) const;
    void GetCursorPos(double* xpos, double* ypos) const;
    void SetInputMode(int mode, int value) const;
    
    // Callback registration
    void SetKeyCallback(KeyCallback callback);
    void SetCursorPosCallback(CursorPosCallback callback);
    void SetMouseButtonCallback(MouseButtonCallback callback);
    void SetScrollCallback(ScrollCallback callback);
    void SetFramebufferSizeCallback(FramebufferSizeCallback callback);
    
    // Get the native GLFW window handle
    void* GetNativeWindow() const;
    
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