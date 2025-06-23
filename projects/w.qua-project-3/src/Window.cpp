/**
 * @class Window
 * @brief OpenGL rendering window and application context.
 *
 * This class manages the creation and lifecycle of an OpenGL rendering window,
 * including initialization, event handling, and the main application loop.
 */

#include "Window.hpp"
#include "Keybinds.hpp"
#include <GLFW/glfw3.h> // as required by task 1

// Static map to associate GLFW windows with Window instances
static std::unordered_map<GLFWwindow*, Window*> windowMap;

Window::Window(int width, int height, const std::string& title)
    : m_Width(width), m_Height(height), m_Title(title), m_Window(nullptr)
{
    static bool glfwInitialized = false;
    if (!glfwInitialized) 
    {
        if (!glfwInit()) 
        {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        glfwInitialized = true;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 

    m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_Window) 
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    // Register this window instance in the map
    windowMap[m_Window] = this;

    // Setup callbacks to route through this class
    glfwSetKeyCallback(m_Window, KeyCallbackWrapper);
    glfwSetCursorPosCallback(m_Window, CursorPosCallbackWrapper);
    glfwSetMouseButtonCallback(m_Window, MouseButtonCallbackWrapper);
    glfwSetScrollCallback(m_Window, ScrollCallbackWrapper);
    glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallbackWrapper);
}

Window::~Window() 
{
    if (m_Window) 
    {
        windowMap.erase(m_Window);
        glfwDestroyWindow(m_Window);
        
        // If this is the last window, terminate GLFW
        if (windowMap.empty()) 
        {
            glfwTerminate();
        }
    }
}

GLFWwindow* Window::GetHandle() const 
{
    return m_Window;
}

bool Window::ShouldClose() const 
{
    return glfwWindowShouldClose(m_Window);
}

void Window::SetShouldClose(bool value) const 
{
    glfwSetWindowShouldClose(m_Window, value);
}

void Window::PollEvents() const 
{
    glfwPollEvents();
}

void Window::SwapBuffers() const 
{
    glfwSwapBuffers(m_Window);
}

void Window::SetTitle(const std::string& title)
{
    m_Title = title;
    glfwSetWindowTitle(m_Window, title.c_str());
}

bool Window::IsKeyPressed(int keyCode) const 
{
    return glfwGetKey(m_Window, keyCode) == GLFW_PRESS;
}

bool Window::IsMouseButtonPressed(int button) const
{
    return glfwGetMouseButton(m_Window, button) == GLFW_PRESS;
}

void Window::GetCursorPos(double* xpos, double* ypos) const
{
    glfwGetCursorPos(m_Window, xpos, ypos);
}

void Window::SetInputMode(int mode, int value) const 
{
    glfwSetInputMode(m_Window, mode, value);
}

void Window::MakeContextCurrent() const 
{
    glfwMakeContextCurrent(m_Window);
}

int Window::GetWidth() const 
{
    return m_Width;
}

int Window::GetHeight() const 
{
    return m_Height;
}

double Window::GetTime() const
{
    return glfwGetTime();
}

void Window::SetKeyCallback(std::function<void(int, int, int, int)> callback) 
{
    m_KeyCallback = callback;
}

void Window::SetCursorPosCallback(std::function<void(double, double)> callback) 
{
    m_CursorPosCallback = callback;
}

void Window::SetMouseButtonCallback(std::function<void(int, int, int)> callback)
{
    m_MouseButtonCallback = callback;
}

void Window::SetScrollCallback(std::function<void(double, double)> callback)
{
    m_ScrollCallback = callback;
}

void Window::SetFramebufferSizeCallback(std::function<void(int, int)> callback)
{
    m_FramebufferSizeCallback = callback;
}

void* Window::GetNativeWindow() const
{
    return m_Window;
}

// Static callback wrappers
void Window::KeyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    auto it = windowMap.find(window);
    if (it != windowMap.end() && it->second->m_KeyCallback) 
    {
        it->second->m_KeyCallback(key, scancode, action, mods);
    }
}

void Window::CursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos) 
{
    auto it = windowMap.find(window);
    if (it != windowMap.end() && it->second->m_CursorPosCallback) 
    {
        it->second->m_CursorPosCallback(xpos, ypos);
    }
}

void Window::MouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods)
{
    auto it = windowMap.find(window);
    if (it != windowMap.end() && it->second->m_MouseButtonCallback)
    {
        it->second->m_MouseButtonCallback(button, action, mods);
    }
}

void Window::ScrollCallbackWrapper(GLFWwindow* window, double xoffset, double yoffset)
{
    auto it = windowMap.find(window);
    if (it != windowMap.end() && it->second->m_ScrollCallback)
    {
        it->second->m_ScrollCallback(xoffset, yoffset);
    }
}

void Window::FramebufferSizeCallbackWrapper(GLFWwindow* window, int width, int height)
{
    auto it = windowMap.find(window);
    if (it != windowMap.end())
    {
        // Update the stored dimensions
        it->second->m_Width = width;
        it->second->m_Height = height;
        
        // Call the user callback if registered
        if (it->second->m_FramebufferSizeCallback)
        {
            it->second->m_FramebufferSizeCallback(width, height);
        }
    }
} 