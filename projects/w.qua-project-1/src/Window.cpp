#include "../include/Window.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

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
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for macOS

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

bool Window::IsKeyPressed(int keyCode) const 
{
    return glfwGetKey(m_Window, keyCode) == GLFW_PRESS;
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

void Window::SetKeyCallback(std::function<void(int, int, int, int)> callback) 
{
    m_KeyCallback = callback;
}

void Window::SetCursorPosCallback(std::function<void(double, double)> callback) 
{
    m_CursorPosCallback = callback;
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