#include "CubeRenderer.hpp"
#include "Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>

CubeRenderer::CubeRenderer(const glm::vec3& center, const glm::vec3& size, const glm::vec3& color)
    : m_Center(center), m_Size(size), m_Color(color)
{
}

CubeRenderer::~CubeRenderer()
{
    CleanUp();
}

void CubeRenderer::Initialize(const std::shared_ptr<Shader>& shader)
{
    m_Shader = shader;
    
    // Create buffer with cube vertices
    auto vertices = CreateVertices();
    
    // Setup the buffer with vertices
    m_Buffer.Setup(vertices);
}

void CubeRenderer::Render(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    if (!m_Shader)
        return;
        
    m_Shader->Use();
    
    // Remove the additional scaling that caused the mismatch
    glm::mat4 cubeModelMatrix = modelMatrix;
    
    // Set matrices
    m_Shader->SetMat4("model", cubeModelMatrix);
    m_Shader->SetMat4("view", viewMatrix);
    m_Shader->SetMat4("projection", projectionMatrix);
    
    // Set material
    m_Shader->SetVec3("material.ambientColor", m_Material.m_AmbientColor);
    m_Shader->SetFloat("material.ambientIntensity", m_Material.m_AmbientIntensity);
    m_Shader->SetVec3("material.diffuseColor", m_Material.m_DiffuseColor);
    m_Shader->SetFloat("material.diffuseIntensity", m_Material.m_DiffuseIntensity);
    m_Shader->SetVec3("material.specularColor", m_Material.m_SpecularColor);
    m_Shader->SetFloat("material.specularIntensity", m_Material.m_SpecularIntensity);
    m_Shader->SetFloat("material.shininess", m_Material.m_Shininess);
    
    // Bind buffer
    m_Buffer.Bind();
    
    // Draw cube triangles (36 vertices for 12 triangles)
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Buffer.GetVertexCount()));
    
    // Unbind
    m_Buffer.Unbind();
}

void CubeRenderer::CleanUp()
{
    // Buffer destructor will handle cleanup
}

void CubeRenderer::SetCenter(const glm::vec3& center)
{
    m_Center = center;
}

glm::vec3 CubeRenderer::GetCenter() const
{
    return m_Center;
}

void CubeRenderer::SetSize(const glm::vec3& size)
{
    m_Size = size;
}

glm::vec3 CubeRenderer::GetSize() const
{
    return m_Size;
}

void CubeRenderer::SetColor(const glm::vec3& color)
{
    m_Color = color;
    
    // Recreate vertices if buffer is setup
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

glm::vec3 CubeRenderer::GetColor() const
{
    return m_Color;
}

std::vector<Vertex> CubeRenderer::CreateVertices()
{
    std::vector<Vertex> vertices;

    glm::vec3 halfSize = glm::vec3(1.0f);  // Unit cube [-1, 1] in all dimensions
    
    // Vertex positions for a unit cube
    const glm::vec3 pos[8] = 
    {
        glm::vec3(-0.5f, -0.5f, -0.5f), // 0: bottom-left-back
        glm::vec3( 0.5f, -0.5f, -0.5f), // 1: bottom-right-back
        glm::vec3( 0.5f,  0.5f, -0.5f), // 2: top-right-back
        glm::vec3(-0.5f,  0.5f, -0.5f), // 3: top-left-back
        glm::vec3(-0.5f, -0.5f,  0.5f), // 4: bottom-left-front
        glm::vec3( 0.5f, -0.5f,  0.5f), // 5: bottom-right-front
        glm::vec3( 0.5f,  0.5f,  0.5f), // 6: top-right-front
        glm::vec3(-0.5f,  0.5f,  0.5f)  // 7: top-left-front
    };
    
    // Normals for each face of the cube
    const glm::vec3 normals[6] = 
    {
        glm::vec3( 0.0f,  0.0f, -1.0f), // Back
        glm::vec3( 0.0f,  0.0f,  1.0f), // Front
        glm::vec3(-1.0f,  0.0f,  0.0f), // Left
        glm::vec3( 1.0f,  0.0f,  0.0f), // Right
        glm::vec3( 0.0f, -1.0f,  0.0f), // Bottom
        glm::vec3( 0.0f,  1.0f,  0.0f)  // Top
    };
    
    // Indices for the 12 triangles (6 faces with 2 triangles each)
    const int indices[36] = {
        // Back face
        0, 1, 2, 0, 2, 3,
        // Front face
        4, 7, 6, 4, 6, 5,
        // Left face
        0, 3, 7, 0, 7, 4,
        // Right face
        1, 5, 6, 1, 6, 2,
        // Bottom face
        0, 4, 5, 0, 5, 1,
        // Top face
        3, 2, 6, 3, 6, 7
    };
    
    // Create triangles
    for (int i = 0; i < 36; i += 3) 
    {
        // Get normal for this face
        int faceIndex = i / 6;
        glm::vec3 normal = normals[faceIndex];
        
        // Create triangle vertices with default UV coordinates
        Vertex v1 = { pos[indices[i]], m_Color, normal, glm::vec2(0.0f, 0.0f) };
        Vertex v2 = { pos[indices[i+1]], m_Color, normal, glm::vec2(1.0f, 0.0f) };
        Vertex v3 = { pos[indices[i+2]], m_Color, normal, glm::vec2(0.5f, 1.0f) };
        
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
    }
    
    return vertices;
} 