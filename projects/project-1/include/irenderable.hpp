#pragma once    

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class IRenderable {
public:
    virtual ~IRenderable() = default;

    /**
     * @brief Initialize the renderable object (shaders, buffers, etc.).
     */
    virtual void initialize() = 0;

    /**
     * @brief Render the object with a provided transformation matrix.
     * @param modelMat The model transformation matrix (position, rotation, scale).
     */
    virtual void render(const glm::mat4& modelMat) = 0;

    /**
     * @brief Cleanup resources (e.g., shaders, buffers).
     */
    virtual void cleanUp() = 0;
};
