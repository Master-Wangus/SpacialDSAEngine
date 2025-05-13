#include "../include/Cube.hpp"
#include "../include/Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Cube::Cube(const glm::vec3& position, float size, const glm::vec3& color)
    : m_position(position),
      m_size(size),
      m_color(color),
      m_material(Material()),
      m_modelMatrix(glm::mat4(1.0f)),
      m_shader(nullptr)
{
    updateModelMatrix();
}

Cube::~Cube() 
{
    // Buffer will be cleaned up by its destructor
}

void Cube::initialize(const std::shared_ptr<Shader>& shader) 
{
    m_shader = shader;
    
    // Create vertices and set up buffer
    std::vector<Vertex> vertices = createVertices();
    m_buffer.setup(vertices);
}

void Cube::render(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) 
{
    if (!m_shader) return;
    
    m_shader->use();
    
    // Set transformation matrices
    m_shader->setMat4("model", m_modelMatrix);
    m_shader->setMat4("view", viewMatrix);
    m_shader->setMat4("projection", projMatrix);
    
    // Set material properties
    m_shader->setVec3("material.ambient", m_material.ambient);
    m_shader->setVec3("material.diffuse", m_material.diffuse);
    m_shader->setVec3("material.specular", m_material.specular);
    m_shader->setFloat("material.shininess", m_material.shininess);
    
    // Bind VAO and draw
    m_buffer.bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_buffer.getVertexCount()));
    m_buffer.unbind();
}

AABB Cube::getAABB() const 
{
    // Half of the cube's size
    float halfSize = m_size * 0.5f;
    
    // Create AABB centered at the cube's position with half-extents
    return AABB(m_position, glm::vec3(halfSize));
}

BoundingSphere Cube::getBoundingSphere() const 
{
    // For a cube, the radius of the bounding sphere is half the diagonal length
    float radius = m_size * 0.5f * std::sqrt(3.0f);
    
    // Create bounding sphere centered at the cube's position
    return BoundingSphere(m_position, radius);
}

void Cube::setPosition(const glm::vec3& position) 
{
    m_position = position;
    updateModelMatrix();
}

glm::vec3 Cube::getPosition() const 
{
    return m_position;
}

void Cube::setMaterial(const Material& material) 
{
    m_material = material;
}

std::vector<Vertex> Cube::createVertices() 
{
    std::vector<Vertex> vertices;
    
    // Generate normalized vertex positions for a cube centered at the origin
    float halfSize = 0.5f;
    
    // Define the 8 corners of the cube
    glm::vec3 corners[8] = 
    {
        glm::vec3(-halfSize, -halfSize, -halfSize), // 0: left-bottom-back
        glm::vec3( halfSize, -halfSize, -halfSize), // 1: right-bottom-back
        glm::vec3( halfSize,  halfSize, -halfSize), // 2: right-top-back
        glm::vec3(-halfSize,  halfSize, -halfSize), // 3: left-top-back
        glm::vec3(-halfSize, -halfSize,  halfSize), // 4: left-bottom-front
        glm::vec3( halfSize, -halfSize,  halfSize), // 5: right-bottom-front
        glm::vec3( halfSize,  halfSize,  halfSize), // 6: right-top-front
        glm::vec3(-halfSize,  halfSize,  halfSize)  // 7: left-top-front
    };
    
    // Define the normals for each face
    glm::vec3 normals[6] = 
    {
        glm::vec3( 0.0f,  0.0f, -1.0f), // back
        glm::vec3( 0.0f,  0.0f,  1.0f), // front
        glm::vec3(-1.0f,  0.0f,  0.0f), // left
        glm::vec3( 1.0f,  0.0f,  0.0f), // right
        glm::vec3( 0.0f, -1.0f,  0.0f), // bottom
        glm::vec3( 0.0f,  1.0f,  0.0f)  // top
    };
    
    // Define texture coordinates for each vertex of a face
    glm::vec2 texCoords[4] = 
    {
        glm::vec2(0.0f, 0.0f), // bottom-left
        glm::vec2(1.0f, 0.0f), // bottom-right
        glm::vec2(1.0f, 1.0f), // top-right
        glm::vec2(0.0f, 1.0f)  // top-left
    };
    
    // Helper to add a face to the vertices vector
    auto addFace = [&](int v0, int v1, int v2, int v3, int normalIndex) 
    {
        Vertex vertex;
        
        // First triangle (v0, v1, v2)
        vertex.position = corners[v0];
        vertex.color = m_color;
        vertex.normal = normals[normalIndex];
        vertex.uv = texCoords[0];
        vertices.push_back(vertex);
        
        vertex.position = corners[v1];
        vertex.uv = texCoords[1];
        vertices.push_back(vertex);
        
        vertex.position = corners[v2];
        vertex.uv = texCoords[2];
        vertices.push_back(vertex);
        
        // Second triangle (v0, v2, v3)
        vertex.position = corners[v0];
        vertex.uv = texCoords[0];
        vertices.push_back(vertex);
        
        vertex.position = corners[v2];
        vertex.uv = texCoords[2];
        vertices.push_back(vertex);
        
        vertex.position = corners[v3];
        vertex.uv = texCoords[3];
        vertices.push_back(vertex);
    };
    
    // Add the 6 faces of the cube
    addFace(0, 1, 2, 3, 0); // back face
    addFace(4, 5, 6, 7, 1); // front face
    addFace(0, 3, 7, 4, 2); // left face
    addFace(1, 5, 6, 2, 3); // right face
    addFace(0, 1, 5, 4, 4); // bottom face
    addFace(3, 2, 6, 7, 5); // top face
    
    return vertices;
}

void Cube::updateModelMatrix() 
{
    // Build model matrix from translation
    m_modelMatrix = glm::translate(glm::mat4(1.0f), m_position);
    
    // Apply scaling based on cube size
    m_modelMatrix = glm::scale(m_modelMatrix, glm::vec3(m_size));
} 