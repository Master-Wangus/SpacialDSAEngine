#include "../include/Buffer.hpp"
#include <iostream>

Buffer::Buffer() : m_vao(0), m_vbo(0), m_vertexCount(0) 
{}

Buffer::~Buffer() 
{
    CleanUp();
}

void Buffer::Setup(const std::vector<Vertex>& vertices) 
{
    // Clean up any existing buffers
    CleanUp();
    
    // Create new buffers with the provided vertices
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
    // Delete buffer objects if they exist
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    
    m_vertexCount = 0;
} 