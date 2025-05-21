#include "pch.h"
#include "Window.hpp"
#include "Shader.hpp"
#include "Registry.hpp"
#include "Systems.hpp"
#include "InputSystem.hpp"
#include "ImGuiManager.hpp"

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
        
        // Initialize GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) 
        {
            throw std::runtime_error("Failed to initialize GLEW");
        }
        
        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        
        // Create shader program
        auto shader = std::make_shared<Shader>(
            "../projects/w.qua-project-1/shaders/my-project-1.vert",
            "../projects/w.qua-project-1/shaders/my-project-1.frag"
        );
        
        // Initialize systems
        Registry registry;
        Systems::InitializeSystems(registry, window, shader);
        
        // Initialize ImGui
        ImGuiManager imguiManager(window);
        imguiManager.Init();
        
        // Subscribe to escape key to close the application
        Systems::g_InputSystem->SubscribeToKey(Window::KEY_ESCAPE, 
            [&window](int key, int scancode, int action, int mods) {
                if (action == Window::PRESS) {
                    window.SetShouldClose(true);
                }
            });
        
        // Game loop variables
        float lastFrame = 0.0f;
        
        // MAIN LOOP
        while (!window.ShouldClose()) 
        {
            // Time calculations
            auto currentFrame = (float)window.GetTime();
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            
            // Update systems with delta time
            Systems::UpdateSystems(registry, window, deltaTime);
            
            // Clear the screen
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Render the scene
            Systems::RenderSystems(registry, window);
            
            // ImGui New Frame
            imguiManager.NewFrame();
            imguiManager.RenderMainWindow(registry);
            imguiManager.Render();
            
            // Swap buffers and poll events
            window.SwapBuffers();
            window.PollEvents();
        }
        
        // Cleanup resources
        imguiManager.Shutdown();
        Systems::ShutdownSystems(registry);
        
        return 0;
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
} 