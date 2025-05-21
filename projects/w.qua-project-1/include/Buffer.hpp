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

    Buffer();
    ~Buffer();

    // Vertex buffer methods
    void Setup(const std::vector<Vertex>& vertices);
    void Bind() const;
    void Unbind() const;
    size_t GetVertexCount() const;
    void UpdateVertices(const std::vector<Vertex>& vertices);

    // Static methods 
    static GLuint CreateUniformBuffer(size_t size, GLuint bindingPoint);
    static void UpdateUniformBuffer(GLuint ubo, const void* data, size_t size, size_t offset = 0);
    static void BindUniformBuffer(GLuint ubo, GLuint bindingPoint);
    static void DeleteUniformBuffer(GLuint ubo);

private:
    GLuint m_vao;         ///< Vertex Array Object ID
    GLuint m_vbo;         ///< Vertex Buffer Object ID
    size_t m_vertexCount; ///< Number of vertices in the buffer
    std::unordered_map<GLuint, GLuint> m_uniformBuffers; ///< Map of UBO IDs to binding points

    void CreateBuffers(const std::vector<Vertex>& vertices);
    void CleanUp();
}; 