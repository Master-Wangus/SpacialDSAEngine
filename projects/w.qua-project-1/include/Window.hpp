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

private:
    GLFWwindow* m_window;         ///< The GLFW window handle
    int m_width;                  ///< Window width
    int m_height;                 ///< Window height
    std::string m_title;          ///< Window title

    // Callback storage
    std::function<void(int, int, int, int)> m_keyCallback;
    std::function<void(double, double)> m_cursorPosCallback;
    
    // Static callback wrappers for GLFW (needed to map to member functions)
    static void KeyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void CursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos);
}; 