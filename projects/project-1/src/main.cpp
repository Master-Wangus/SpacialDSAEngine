/**
 * @file main.cpp
 * @brief Entry point for a simple OpenGL application rendering a 3D cube.
 *
 * This program initializes an OpenGL context using GLFW and GLEW, sets up a
 * basic rendering scene, and continuously updates and renders a cube object.
 */

 // Scene class with basic lifecycle and renderable object management
#include <cube.hpp>   ///< Contains the Cube class implementing a colored rotating cube.
#include <irenderable.hpp>
#include <memory>

/**
 * @brief Main function that sets up the OpenGL window and rendering loop.
 * @return Exit status (0 if successful).
 */
int main() {
    // Initialize the GLFW library
    if (!glfwInit()) return -1;

    // Configure the OpenGL context version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for macOS

    // Create the main application window
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Render Cube", nullptr, nullptr);
    if (!window) return -1;

    // Set the current OpenGL context to the newly created window
    glfwMakeContextCurrent(window);

    // Initialize GLEW to load OpenGL functions
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) return -1;

    // Enable depth testing for proper 3D rendering
    glEnable(GL_DEPTH_TEST);

    // Ensure ESC key can be captured properly
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Create the scene manager and add a cube object
    std::vector<std::shared_ptr<IRenderable>> _objects; ///< Container for renderable objects.

    //Scene scene;
    auto cube = std::make_shared<Cube>();
    _objects.push_back(cube);

    //scene.addObject(cube);
    //scene.initialize(); ///< Initialize all objects in the scene
    for (auto& obj : _objects) {
        obj->initialize();
    }

    // Apply rotation to model matrix
    static float angleOfRotation = 0.0f; ///< Rotation angle for the cube

    // Main application loop
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Set background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear frame and depth
        
        // update system
        angleOfRotation += 0.01f; // Increment rotation angle

        // Render each object in the scene
        for (auto& obj : _objects) {
            glm::mat4 modelMat = glm::rotate(angleOfRotation, glm::vec3(1.0f, 1.0f, 0.0f));
            obj->render(modelMat);
        }

        glfwSwapBuffers(window);  ///< Swap front and back buffers
        glfwPollEvents();         ///< Process input and events
    }

    // Cleanly shut down and exit
    glfwTerminate();
    return 0;
}