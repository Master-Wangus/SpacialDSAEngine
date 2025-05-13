#pragma once

#include <functional>
#include <string>
#include <stdexcept>

// Forward declaration of GLFW types to avoid including GLFW header here
struct GLFWwindow;

/**
 * @class Window
 * @brief Encapsulates GLFW window functionality with input handling and error checking
 */
class Window {
public:
    /**
     * @brief Constructor that creates a GLFW window with specified dimensions and title
     * @param width Window width in pixels
     * @param height Window height in pixels
     * @param title Window title
     * @throws std::runtime_error if window creation fails
     */
    Window(int width, int height, const std::string& title);

    /**
     * @brief Destructor that cleans up GLFW resources
     */
    ~Window();

    /**
     * @brief Get window handle (only for internal use)
     * @return The underlying GLFW window handle
     */
    GLFWwindow* getHandle() const;

    /**
     * @brief Check if the window should close
     * @return True if the window should close
     */
    bool shouldClose() const;
    
    /**
     * @brief Process pending input events
     */
    void pollEvents() const;
    
    /**
     * @brief Swap back and front buffers to display rendered content
     */
    void swapBuffers() const;
    
    /**
     * @brief Check if a specific key is pressed
     * @param keyCode The GLFW key code to check
     * @return True if the key is pressed
     */
    bool isKeyPressed(int keyCode) const;
    
    /**
     * @brief Set input mode (e.g., sticky keys, cursor options)
     * @param mode The input mode to set
     * @param value The value to set
     */
    void setInputMode(int mode, int value) const;

    /**
     * @brief Make this window's context current for OpenGL operations
     */
    void makeContextCurrent() const;

    /**
     * @brief Get window width
     * @return Window width in pixels
     */
    int getWidth() const;
    
    /**
     * @brief Get window height
     * @return Window height in pixels
     */
    int getHeight() const;

    /**
     * @brief Set a key callback function
     * @param callback The function to call when a key is pressed
     */
    void setKeyCallback(std::function<void(int, int, int, int)> callback);
    
    /**
     * @brief Set a mouse position callback function
     * @param callback The function to call when the mouse is moved
     */
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