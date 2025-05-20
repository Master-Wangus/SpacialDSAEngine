#include "../include/Window.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

// Static map to associate GLFW windows with Window instances
static std::unordered_map<GLFWwindow*, Window*> windowMap;

Window::Window(int width, int height, const std::string& title)
    : m_width(width), m_height(height), m_title(title), m_window(nullptr)
{
    // Initialize GLFW library if it hasn't been done yet
    static bool glfwInitialized = false;
    if (!glfwInitialized) 
    {
        if (!glfwInit()) 
        {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        glfwInitialized = true;
    }

    // Configure OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for macOS

    // Create window
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) 
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    // Register this window instance in the map
    windowMap[m_window] = this;

    // Setup callbacks to route through this class
    glfwSetKeyCallback(m_window, KeyCallbackWrapper);
    glfwSetCursorPosCallback(m_window, CursorPosCallbackWrapper);
}

Window::~Window() 
{
    if (m_window) 
    {
        // Remove from map
        windowMap.erase(m_window);
        
        // Destroy window
        glfwDestroyWindow(m_window);
        
        // If this is the last window, terminate GLFW
        if (windowMap.empty()) 
        {
            glfwTerminate();
        }
    }
}

GLFWwindow* Window::GetHandle() const 
{
    return m_window;
}

bool Window::ShouldClose() const 
{
    return glfwWindowShouldClose(m_window);
}

void Window::SetShouldClose(bool value) const 
{
    glfwSetWindowShouldClose(m_window, value);
}

void Window::PollEvents() const 
{
    glfwPollEvents();
}

void Window::SwapBuffers() const 
{
    glfwSwapBuffers(m_window);
}

bool Window::IsKeyPressed(int keyCode) const 
{
    return glfwGetKey(m_window, keyCode) == GLFW_PRESS;
}

void Window::SetInputMode(int mode, int value) const 
{
    glfwSetInputMode(m_window, mode, value);
}

void Window::MakeContextCurrent() const 
{
    glfwMakeContextCurrent(m_window);
}

int Window::GetWidth() const 
{
    return m_width;
}

int Window::GetHeight() const 
{
    return m_height;
}

void Window::SetKeyCallback(std::function<void(int, int, int, int)> callback) 
{
    m_keyCallback = callback;
}

void Window::SetCursorPosCallback(std::function<void(double, double)> callback) 
{
    m_cursorPosCallback = callback;
}

// Static callback wrappers
void Window::KeyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    auto it = windowMap.find(window);
    if (it != windowMap.end() && it->second->m_keyCallback) 
    {
        it->second->m_keyCallback(key, scancode, action, mods);
    }
}

void Window::CursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos) 
{
    auto it = windowMap.find(window);
    if (it != windowMap.end() && it->second->m_cursorPosCallback) 
    {
        it->second->m_cursorPosCallback(xpos, ypos);
    }
} 