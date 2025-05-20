#include "../include/Cube.hpp"
#include "../include/Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Cube::Cube(const glm::vec3& color, float size)
    : m_Size(size),
      m_Color(color)
{
    // Initialize with default white material
    // No need to update model matrix as it's now handled by the TransformComponent
}

Cube::~Cube() 
{
    CleanUp();
}

void Cube::Initialize(const std::shared_ptr<Shader>& shader) 
{
    m_shader = shader;
    
    // Create vertices and set up buffer
    std::vector<Vertex> vertices = CreateVertices();
    m_buffer.Setup(vertices);
}

void Cube::Render(const glm::mat4& modelMatrix, 
                 const glm::mat4& viewMatrix, 
                 const glm::mat4& projectionMatrix)
{
    if (!m_shader) return;
    
    m_shader->Use();
    
    // Set transformation matrices
    m_shader->SetMat4("model", modelMatrix);
    m_shader->SetMat4("view", viewMatrix);
    m_shader->SetMat4("projection", projectionMatrix);
    
    // Set camera position for specular lighting
    glm::vec3 cameraPos = glm::vec3(viewMatrix[3][0], viewMatrix[3][1], viewMatrix[3][2]);
    m_shader->SetVec3("viewPos", -cameraPos);
    
    // Bind uniform blocks to their respective binding points
    GLuint materialBlockIndex = glGetUniformBlockIndex(m_shader->GetID(), "Material");
    GLuint lightBlockIndex = glGetUniformBlockIndex(m_shader->GetID(), "DirectionalLight");
    
    if (materialBlockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(m_shader->GetID(), materialBlockIndex, 1); // Binding point 1 for Material
        
        // Update material data in UBO
        static GLuint materialUBO = 0;
        if (materialUBO == 0) {
            glGenBuffers(1, &materialUBO);
            glBindBuffer(GL_UNIFORM_BUFFER, materialUBO);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), nullptr, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, materialUBO); // Bind to binding point 1
        }
        
        glBindBuffer(GL_UNIFORM_BUFFER, materialUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &m_Material);
    }
    
    if (lightBlockIndex != GL_INVALID_INDEX) {
        glUniformBlockBinding(m_shader->GetID(), lightBlockIndex, 0); // Binding point 0 for Light
    }
    
    // Bind VAO and draw
    m_buffer.Bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_buffer.GetVertexCount()));
    m_buffer.Unbind();
}

void Cube::CleanUp()
{
    // Buffer cleans up with destructor
    m_shader.reset();
}

void Cube::SetColor(const glm::vec3& color)
{
    m_Color = color;
    // Regenerate vertices with new color
    std::vector<Vertex> vertices = CreateVertices();
    m_buffer.UpdateVertices(vertices);
}

glm::vec3 Cube::GetColor() const
{
    return m_Color;
}

float Cube::GetSize() const
{
    return m_Size;
}

std::vector<Vertex> Cube::CreateVertices() 
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
        vertex.m_Position = corners[v0];
        vertex.m_Color = m_Color;
        vertex.m_Normal = normals[normalIndex];
        vertex.m_UV = texCoords[0];
        vertices.push_back(vertex);
        
        vertex.m_Position = corners[v1];
        vertex.m_UV = texCoords[1];
        vertices.push_back(vertex);
        
        vertex.m_Position = corners[v2];
        vertex.m_UV = texCoords[2];
        vertices.push_back(vertex);
        
        // Second triangle (v0, v2, v3)
        vertex.m_Position = corners[v0];
        vertex.m_UV = texCoords[0];
        vertices.push_back(vertex);
        
        vertex.m_Position = corners[v2];
        vertex.m_UV = texCoords[2];
        vertices.push_back(vertex);
        
        vertex.m_Position = corners[v3];
        vertex.m_UV = texCoords[3];
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