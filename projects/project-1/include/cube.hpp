#pragma once

#include <IRenderable.hpp>
#include <vector>
#include <shader.hpp>

/// Macro for basic OpenGL error checking.
#define _GET_GL_ERROR { GLenum err = glGetError(); std::cout << "[OpenGL Error] " << glewGetErrorString(err) << std::endl; }

/**
 * @class Cube
 * @brief Represents a 3D cube renderable with vertex colors and transformation logic.
 *
 * This class implements the IRenderable interface, encapsulating geometry data,
 * shader program, and OpenGL buffer setup for a colored rotating cube.
 */
class Cube : public IRenderable {
public:
    /**
     * @brief Constructs a cube with given window dimensions (used for transforms or viewport logic).
     * @param windowWidth Width of the rendering window.
     * @param windowHeight Height of the rendering window.
     */
    Cube();

    /**
     * @brief Destructor that ensures proper cleanup of OpenGL resources.
     */
    ~Cube();

    /**
     * @brief Initializes GPU buffers and loads shaders.
     */
    void initialize() override;

    /**
     * @brief Renders the cube geometry with colors.
     */
    void render(const glm::mat4& modelMat) override;

    /**
     * @brief Cleans up GPU resources (buffers, shaders).
     */
    void cleanUp();

private:
    /**
     * @brief Generates vertex buffers and sets up VAO for the cube.
     */
    void SetupBuffers();

    GLuint programID;                 ///< ID of the compiled shader program.
    GLuint vertexbuffer;             ///< Vertex buffer object ID.
    GLuint VertexArrayID;            ///< Vertex array object ID.
    std::vector<GLfloat> geometryBuffer; ///< Interleaved vertex data (positions and colors).
};
