/**
 * @class Buffer
 * @brief OpenGL vertex buffer wrapper for 3D geometry data management.
 *
 * This class encapsulates the OpenGL vertex buffer object (VBO) and vertex array object (VAO)
 * operations, providing a simplified interface for managing 3D geometry data.
 */

#pragma once

#include "pch.h"

struct Vertex 
{
    glm::vec3 m_Position; ///< (x, y, z) position
    glm::vec3 m_Color;    ///< (r, g, b) color
    glm::vec3 m_Normal;   ///< (x, y, z) normal vector
    glm::vec2 m_UV;       ///< (u, v) texture coordinates
};


class Buffer 
{
public:

    /**
     * @brief Constructs a new buffer object.
     */
    Buffer();
    
    /**
     * @brief Destructor that cleans up buffer resources.
     */
    ~Buffer();

    // Vertex buffer methods
    /**
     * @brief Sets up the buffer with vertex data.
     * @param vertices Vector of vertex data to upload to the buffer
     */
    void Setup(const std::vector<Vertex>& vertices);
    
    /**
     * @brief Binds the vertex array object for rendering.
     */
    void Bind() const;
    
    /**
     * @brief Unbinds the vertex array object.
     */
    void Unbind() const;
    
    /**
     * @brief Gets the number of vertices in the buffer.
     * @return Number of vertices stored in the buffer
     */
    size_t GetVertexCount() const;
    
    /**
     * @brief Updates the vertex data in the buffer.
     * @param vertices New vertex data to upload
     */
    void UpdateVertices(const std::vector<Vertex>& vertices);

    // Static methods 
    /**
     * @brief Creates a uniform buffer object.
     * @param size Size of the buffer in bytes
     * @param bindingPoint Binding point for the uniform buffer
     * @return OpenGL uniform buffer object ID
     */
    static GLuint CreateUniformBuffer(size_t size, GLuint bindingPoint);
    
    /**
     * @brief Updates data in a uniform buffer object.
     * @param ubo Uniform buffer object ID
     * @param data Pointer to the data to upload
     * @param size Size of the data in bytes
     * @param offset Offset in the buffer where to start writing
     */
    static void UpdateUniformBuffer(GLuint ubo, const void* data, size_t size, size_t offset = 0);
    
    /**
     * @brief Binds a uniform buffer to a binding point.
     * @param ubo Uniform buffer object ID
     * @param bindingPoint Binding point to bind to
     */
    static void BindUniformBuffer(GLuint ubo, GLuint bindingPoint);
    
    /**
     * @brief Deletes a uniform buffer object.
     * @param ubo Uniform buffer object ID to delete
     */
    static void DeleteUniformBuffer(GLuint ubo);

private:
    GLuint m_vao;         ///< Vertex Array Object ID
    GLuint m_vbo;         ///< Vertex Buffer Object ID
    size_t m_vertexCount; ///< Number of vertices in the buffer
    std::unordered_map<GLuint, GLuint> m_uniformBuffers; ///< Map of UBO IDs to binding points

    /**
     * @brief Creates and initializes OpenGL buffer objects.
     * @param vertices Vertex data to upload to the buffer
     */
    void CreateBuffers(const std::vector<Vertex>& vertices);
    
    /**
     * @brief Cleans up OpenGL buffer resources.
     */
    void CleanUp();
}; 