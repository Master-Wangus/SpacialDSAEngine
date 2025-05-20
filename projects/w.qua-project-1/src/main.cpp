#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <chrono>

#include "../include/Window.hpp"
#include "../include/Shader.hpp"
#include "../include/Registry.hpp"
#include "../include/Systems.hpp"

// Constants
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;
const char* WINDOW_TITLE = "Geometry Toolbox";

int main() 
{
    try 
    {
        // Create window
        Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
        window.MakeContextCurrent();
        window.SetInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
        // Initialize GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) 
        {
            throw std::runtime_error("Failed to initialize GLEW");
        }
        
        // Configure OpenGL
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        
        // Create shader
        auto shader = std::make_shared<Shader>(
            "../projects/w.qua-project-1/shaders/my-project-1.vert",
            "../projects/w.qua-project-1/shaders/my-project-1.frag"
        );
        
        // Create registry
        Registry registry;
        
        // Initialize systems and set up the scene
        Systems::InitializeSystems(registry, window, shader);
        
        // Timing variables
        float lastFrame = 0.0f;
        
        // Main loop
        while (!window.ShouldClose()) 
        {
            // Calculate delta time
            auto currentFrame = static_cast<float>(glfwGetTime());
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            
            // Check for escape key to close the application
            if (window.IsKeyPressed(GLFW_KEY_ESCAPE)) {
                window.SetShouldClose(true);
            }
            
            // Update all systems
            Systems::UpdateSystems(registry, window, deltaTime);
            
            // Clear buffers
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Render all systems
            Systems::RenderSystems(registry, window);
            
            // Swap buffers and poll events
            window.SwapBuffers();
            window.PollEvents();
        }
        
        // Cleanup
        Systems::ShutdownSystems(registry);
        
        return 0;
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
} 