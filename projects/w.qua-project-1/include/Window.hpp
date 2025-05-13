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
    GLFWwindow* getHandle() const;
    bool shouldClose() const;
    void pollEvents() const;
    void swapBuffers() const;
    bool isKeyPressed(int keyCode) const;
    void setInputMode(int mode, int value) const;
    void makeContextCurrent() const;
    int getWidth() const;
    int getHeight() const;
    void setKeyCallback(std::function<void(int, int, int, int)> callback);
    void setCursorPosCallback(std::function<void(double, double)> callback);

private:
    GLFWwindow* m_window;         ///< The GLFW window handle
    int m_width;                  ///< Window width
    int m_height;                 ///< Window height
    std::string m_title;          ///< Window title

    // Callback storage
    std::function<void(int, int, int, int)> m_keyCallback;
    std::function<void(double, double)> m_cursorPosCallback;
    
    // Static callback wrappers for GLFW (needed to map to member functions)
    static void keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos);
}; 