#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

/**
 * @struct Vertex
 * @brief Structure to store vertex data including position, color, normal, and texture coordinates
 */
struct Vertex {
    glm::vec3 position; ///< (x, y, z) position
    glm::vec3 color;    ///< (r, g, b) color
    glm::vec3 normal;   ///< (x, y, z) normal vector
    glm::vec2 uv;       ///< (u, v) texture coordinates
};

/**
 * @class Buffer
 * @brief Manages OpenGL buffer objects (VAO, VBO) and attribute layout
 */
class Buffer {
public:
    /**
     * @brief Constructor that initializes buffer objects
     */
    Buffer();

    /**
     * @brief Destructor that cleans up buffer objects
     */
    ~Buffer();

    /**
     * @brief Sets up buffer with given vertex data and attribute layout
     * @param vertices Vector of vertex data
     * @param attributeLayout Vector specifying the number of components for each attribute
     *        e.g., {3, 3} means two attributes with 3 components each (position, color)
     */
    void setup(const std::vector<Vertex>& vertices);

    /**
     * @brief Binds the VAO for rendering
     */
    void bind() const;

    /**
     * @brief Unbinds the VAO
     */
    void unbind() const;

    /**
     * @brief Gets the number of vertices in this buffer
     * @return The number of vertices
     */
    size_t getVertexCount() const;
    
    /**
     * @brief Sets vertices and updates buffers (used for dynamic geometry)
     * @param vertices New vertex data
     */
    void updateVertices(const std::vector<Vertex>& vertices);

private:
    GLuint m_vao;         ///< Vertex Array Object ID
    GLuint m_vbo;         ///< Vertex Buffer Object ID
    size_t m_vertexCount; ///< Number of vertices in the buffer

    /**
     * @brief Creates VAO, VBO and sets up attribute pointers
     * @param vertices Vector of vertex data
     */
    void createBuffers(const std::vector<Vertex>& vertices);
    
    /**
     * @brief Releases GPU resources
     */
    void cleanUp();
}; 