#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


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

    void Setup(const std::vector<Vertex>& vertices);
    void Bind() const;
    void Unbind() const;
    size_t GetVertexCount() const;
    void UpdateVertices(const std::vector<Vertex>& vertices);

private:
    GLuint m_vao;         ///< Vertex Array Object ID
    GLuint m_vbo;         ///< Vertex Buffer Object ID
    size_t m_vertexCount; ///< Number of vertices in the buffer

    void CreateBuffers(const std::vector<Vertex>& vertices);
    void CleanUp();
}; 