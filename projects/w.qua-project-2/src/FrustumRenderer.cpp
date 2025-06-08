/**
 * @class FrustumRenderer
 * @brief Renderer for view frustum visualization with wireframe display.
 */

#include "FrustumRenderer.hpp"
#include "Shader.hpp"
#include "Buffer.hpp"
#include <cmath>

FrustumRenderer::FrustumRenderer(const glm::vec3& color)
    : m_Color(color), m_InvViewProjection(1.0f)
{
    // Initialize with default material
    m_Material.m_DiffuseColor = color;
    m_Material.m_AmbientColor = color * 0.2f;
    m_Material.m_SpecularColor = glm::vec3(0.1f);
}

FrustumRenderer::~FrustumRenderer()
{
    CleanUp();
}

void FrustumRenderer::Initialize(const std::shared_ptr<Shader>& shader)
{
    m_Shader = shader;
    UpdateVertices();
}

void FrustumRenderer::Render(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    if (!m_Shader)
        return;

    if (m_FrustumDirty) {
        UpdateVertices();
        m_FrustumDirty = false;
    }

    m_Shader->Use();
    
    m_Shader->SetMat4("model", modelMatrix);
    m_Shader->SetMat4("view", viewMatrix);
    m_Shader->SetMat4("projection", projectionMatrix);
    
    m_Shader->SetBool("disableLighting", true);
    
    m_Buffer.Bind();
    
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_Buffer.GetVertexCount()));
    
    m_Buffer.Unbind();
}

void FrustumRenderer::CleanUp()
{
    // Buffer cleanup is handled automatically in Buffer destructor
}

void FrustumRenderer::UpdateFrustum(const glm::mat4& invViewProjection)
{
    // Only update if the matrix has significantly changed (use tolerance for floating-point comparison)
    // Dosen't affect the flickering issues that much...
    const float epsilon = 1e-6f; // Tolerance for matrix comparison
    bool matricesEqual = true;
    
    for (int i = 0; i < 4 && matricesEqual; ++i) 
    {
        for (int j = 0; j < 4 && matricesEqual; ++j) 
        {
            if (std::abs(m_InvViewProjection[i][j] - invViewProjection[i][j]) > epsilon) {
                matricesEqual = false;
            }
        }
    }
    
    if (!matricesEqual)
    {
        m_InvViewProjection = invViewProjection;
        m_FrustumDirty = true;
    }
}

std::vector<Vertex> FrustumRenderer::CreateFrustumVertices()
{
    std::vector<Vertex> vertices;
    
    // Define the 8 corners of the NDC cube [-1,1] in each axis
    glm::vec4 ndcCorners[8] = 
    {
        // Near plane (z = -1)
        glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f), // Bottom-left-near
        glm::vec4( 1.0f, -1.0f, -1.0f, 1.0f), // Bottom-right-near
        glm::vec4( 1.0f,  1.0f, -1.0f, 1.0f), // Top-right-near
        glm::vec4(-1.0f,  1.0f, -1.0f, 1.0f), // Top-left-near
        
        // Far plane (z = 1)
        glm::vec4(-1.0f, -1.0f,  1.0f, 1.0f), // Bottom-left-far
        glm::vec4( 1.0f, -1.0f,  1.0f, 1.0f), // Bottom-right-far
        glm::vec4( 1.0f,  1.0f,  1.0f, 1.0f), // Top-right-far
        glm::vec4(-1.0f,  1.0f,  1.0f, 1.0f)  // Top-left-far
    };
    
    // Transform NDC corners to world space
    glm::vec3 worldCorners[8];
    for (int i = 0; i < 8; i++) {
        glm::vec4 worldPos = m_InvViewProjection * ndcCorners[i];
        worldPos /= worldPos.w; // Perspective divide
        worldCorners[i] = glm::vec3(worldPos);
    }
    
    // Create line segments for the 12 edges of the frustum
    // Near plane edges (0,1,2,3)
    int edges[24] =
    {
        // Near plane rectangle
        0, 1,  1, 2,  2, 3,  3, 0,
        // Far plane rectangle  
        4, 5,  5, 6,  6, 7,  7, 4,
        // Connecting edges between near and far
        0, 4,  1, 5,  2, 6,  3, 7
    };
    
    // Create vertex data for all edges
    for (int i = 0; i < 24; i++) 
    {
        Vertex vertex;
        vertex.m_Position = worldCorners[edges[i]];
        vertex.m_Color = m_Color;
        vertex.m_Normal = glm::vec3(0.0f, 1.0f, 0.0f); // Dummy normal
        vertex.m_UV = glm::vec2(0.0f);
        vertices.push_back(vertex);
    }
    
    return vertices;
}

void FrustumRenderer::UpdateVertices()
{
    auto vertices = CreateFrustumVertices();
    m_Buffer.Setup(vertices);
} 