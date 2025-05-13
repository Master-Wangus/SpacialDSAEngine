#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


struct Vertex {
    glm::vec3 position; ///< (x, y, z) position
    glm::vec3 color;    ///< (r, g, b) color
    glm::vec3 normal;   ///< (x, y, z) normal vector
    glm::vec2 uv;       ///< (u, v) texture coordinates
};


class Buffer {
public:

    Buffer();
    ~Buffer();

    void setup(const std::vector<Vertex>& vertices);
    void bind() const;
    void unbind() const;
    size_t getVertexCount() const;
    void updateVertices(const std::vector<Vertex>& vertices);

private:
    GLuint m_vao;         ///< Vertex Array Object ID
    GLuint m_vbo;         ///< Vertex Buffer Object ID
    size_t m_vertexCount; ///< Number of vertices in the buffer

    void createBuffers(const std::vector<Vertex>& vertices);
    void cleanUp();
}; 