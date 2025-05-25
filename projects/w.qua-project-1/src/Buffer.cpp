/**
 * @class Buffer
 * @brief OpenGL vertex buffer wrapper for 3D geometry data management.
 *
 * This class encapsulates the OpenGL vertex buffer object (VBO) and vertex array object (VAO)
 * operations, providing a simplified interface for managing 3D geometry data.
 */

#include "Buffer.hpp"

Buffer::Buffer() : m_vao(0), m_vbo(0), m_vertexCount(0) 
{}

Buffer::~Buffer() 
{
    CleanUp();
}

void Buffer::Setup(const std::vector<Vertex>& vertices) 
{
    CleanUp();
    CreateBuffers(vertices);
}

void Buffer::Bind() const 
{
    glBindVertexArray(m_vao);
}

void Buffer::Unbind() const 
{
    glBindVertexArray(0);
}

size_t Buffer::GetVertexCount() const 
{
    return m_vertexCount;
}

void Buffer::UpdateVertices(const std::vector<Vertex>& vertices) 
{
    m_vertexCount = vertices.size();
    
    // Bind VBO and update data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

GLuint Buffer::CreateUniformBuffer(size_t size, GLuint bindingPoint)
{
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);
    return ubo;
}

void Buffer::UpdateUniformBuffer(GLuint ubo, const void* data, size_t size, size_t offset)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    if (data != nullptr) {
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    }
}

void Buffer::BindUniformBuffer(GLuint ubo, GLuint bindingPoint)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);
}

void Buffer::DeleteUniformBuffer(GLuint ubo)
{
    if (ubo != 0) {
        glDeleteBuffers(1, &ubo);
    }
}

void Buffer::CreateBuffers(const std::vector<Vertex>& vertices) 
{
    m_vertexCount = vertices.size();
    
    // Create and bind VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    
    // Create and bind VBO
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // Upload data to VBO
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    
    // Position attribute (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Position));
    
    // Color attribute (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Color));
    
    // Normal attribute (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Normal));
    
    // Texture coordinates attribute (location = 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_UV));
    
    // Unbind VAO to prevent accidental modification
    glBindVertexArray(0);
}

void Buffer::CleanUp() 
{
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    
    // Delete UBOs
    for (const auto& pair : m_uniformBuffers) 
    {
        glDeleteBuffers(1, &pair.first);
    }
    m_uniformBuffers.clear();
    
    m_vertexCount = 0;
} 