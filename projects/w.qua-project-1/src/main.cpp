#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <entt/entt.hpp>

#include "../include/Window.hpp"
#include "../include/Camera.hpp"
#include "../include/Shader.hpp"
#include "../include/Components.hpp"
#include "../include/Systems.hpp"

// Constants
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;
const char* WINDOW_TITLE = "Geometry Toolbox";

int main() 
{
    try {
        // Create window
        Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
        window.makeContextCurrent();
        window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
        // Initialize GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            throw std::runtime_error("Failed to initialize GLEW");
        }
        
        // Configure OpenGL
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        
        // Create shader
        auto shader = std::make_shared<Shader>(
            "projects/w.qua-project-1/shaders/phong.vert",
            "projects/w.qua-project-1/shaders/phong.frag"
        );
        
        // Create registry
        entt::registry registry;
        
        // Create the FPS camera system
        Systems::FPSCameraSystem cameraSystem(registry, window);
        
        // Create light
        Systems::createDirectionalLight(
            registry,
            glm::vec3(-0.2f, -1.0f, -0.3f),
            glm::vec3(0.2f),
            glm::vec3(0.5f),
            glm::vec3(1.0f)
        );
        
        // Create cubes
        Systems::createCubeEntity(registry, glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, glm::vec3(1.0f, 0.5f, 0.31f), shader);
        Systems::createCubeEntity(registry, glm::vec3(2.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.31f, 0.5f, 1.0f), shader);
        Systems::createCubeEntity(registry, glm::vec3(-2.0f, 0.0f, 0.0f), 1.0f, glm::vec3(0.5f, 1.0f, 0.31f), shader);
        Systems::createCubeEntity(registry, glm::vec3(0.0f, 2.0f, 0.0f), 1.0f, glm::vec3(1.0f, 0.31f, 0.5f), shader);
        Systems::createCubeEntity(registry, glm::vec3(0.0f, -2.0f, 0.0f), 1.0f, glm::vec3(0.31f, 1.0f, 0.5f), shader);
        
        // Timing variables
        float lastFrame = 0.0f;
        
        // Main loop
        while (!window.shouldClose()) 
        {
            // Calculate delta time
            auto currentFrame = static_cast<float>(glfwGetTime());
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            
            // Process input and update camera
            cameraSystem.update(deltaTime);
            
            // Clear buffers
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Update transforms
            Systems::updateTransforms(registry);
            
            // Update colliders
            Systems::updateColliders(registry);
            
            // Check for collisions
            Systems::collisionSystem(registry);
            
            // Create a Camera adapter for the renderSystem (for backward compatibility)
            class FPSCameraAdapter : public Camera 
            {
            public:
                FPSCameraAdapter(const Systems::FPSCameraSystem& system, float aspectRatio)
                    : m_system(system), m_aspectRatio(aspectRatio) {}
                
                glm::mat4 getViewMatrix() const 
                {
                    return m_system.getViewMatrix();
                }
                
                glm::mat4 getProjectionMatrix(float aspectRatio) const 
                {
                    return m_system.getProjectionMatrix(aspectRatio);
                }
                
                glm::vec3 getPosition() const 
                {
                    // Use the camera system's position getter
                    return m_system.getPosition();
                }
                
            private:
                const Systems::FPSCameraSystem& m_system;
                float m_aspectRatio;
            };
            
            // Create adapter and render the scene
            FPSCameraAdapter cameraAdapter(cameraSystem, static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT);
            Systems::renderSystem(registry, cameraAdapter);
            
            // Swap buffers and poll events
            window.swapBuffers();
            window.pollEvents();
        }
        
        return 0;
    } catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
} 