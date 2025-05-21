#pragma once

#include <functional>
#include <string>
#include <stdexcept>

struct GLFWwindow;

class Window 
{
public:
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

    Window(int width, int height, const std::string& title);
    ~Window();
    GLFWwindow* GetHandle() const;
    bool ShouldClose() const;
    void SetShouldClose(bool value) const;
    void PollEvents() const;
    void SwapBuffers() const;
    bool IsKeyPressed(int keyCode) const;
    void SetInputMode(int mode, int value) const;
    void MakeContextCurrent() const;
    int GetWidth() const;
    int GetHeight() const;
    float GetTime() const; 
    void SetKeyCallback(std::function<void(int, int, int, int)> callback);
    void SetCursorPosCallback(std::function<void(double, double)> callback);
    void SetMouseButtonCallback(std::function<void(int, int, int)> callback);

private:
    GLFWwindow* m_Window;         ///< The GLFW window handle
    int m_Width;                 
    int m_Height;                 
    std::string m_Title;          

    // Callback storage
    std::function<void(int, int, int, int)> m_KeyCallback;
    std::function<void(double, double)> m_CursorPosCallback;
    std::function<void(int, int, int)> m_MouseButtonCallback;
    
    // Static callback wrappers for GLFW (needed to map to member functions)
    static void KeyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void CursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos);
    static void MouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods);
}; 