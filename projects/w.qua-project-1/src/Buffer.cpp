#include "../include/Buffer.hpp"
#include <iostream>

Buffer::Buffer() : m_vao(0), m_vbo(0), m_vertexCount(0) {
}

Buffer::~Buffer() {
    cleanUp();
}

void Buffer::setup(const std::vector<Vertex>& vertices) {
    // Clean up any existing buffers
    cleanUp();
    
    // Create new buffers with the provided vertices
    createBuffers(vertices);
}

void Buffer::bind() const {
    glBindVertexArray(m_vao);
}

void Buffer::unbind() const {
    glBindVertexArray(0);
}

size_t Buffer::getVertexCount() const {
    return m_vertexCount;
}

void Buffer::updateVertices(const std::vector<Vertex>& vertices) {
    m_vertexCount = vertices.size();
    
    // Bind VBO and update data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

void Buffer::createBuffers(const std::vector<Vertex>& vertices) {
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    // Color attribute (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    
    // Normal attribute (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    
    // Texture coordinates attribute (location = 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    
    // Unbind VAO to prevent accidental modification
    glBindVertexArray(0);
}

void Buffer::cleanUp() {
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