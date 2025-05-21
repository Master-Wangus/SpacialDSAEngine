#pragma once

#include <functional>
#include <string>
#include <stdexcept>

// Forward declaration of GLFW types to avoid including GLFW header here
struct GLFWwindow;

class Window 
{
public:
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