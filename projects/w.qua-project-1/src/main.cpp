#include <GL/glew.h>
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

// Global variables
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Mouse callback function
void mouseCallback(double xpos, double ypos) {
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }
    
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // Reversed since y-coordinates go from bottom to top
    
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);
    
    camera.processMouseMovement(xoffset, yoffset);
}

int main() {
    try {
        // Create window
        Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
        window.makeContextCurrent();
        window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
        // Set mouse callback
        window.setCursorPosCallback(mouseCallback);
        
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
            "../projects/project-1-geometry-toolbox/shaders/phong.vert",
            "../projects/project-1-geometry-toolbox/shaders/phong.frag"
        );
        
        // Create registry
        entt::registry registry;
        
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
        
        // Main loop
        while (!window.shouldClose()) {
            // Calculate delta time
            auto currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            
            // Process input
            camera.update(window, deltaTime);
            
            // Clear buffers
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Update transforms
            Systems::updateTransforms(registry);
            
            // Update colliders
            Systems::updateColliders(registry);
            
            // Check for collisions
            Systems::collisionSystem(registry);
            
            // Render scene
            Systems::renderSystem(registry, camera);
            
            // Swap buffers and poll events
            window.swapBuffers();
            window.pollEvents();
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
} 