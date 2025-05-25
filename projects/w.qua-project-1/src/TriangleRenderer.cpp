#include "TriangleRenderer.hpp"
#include "Shader.hpp"

TriangleRenderer::TriangleRenderer(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& color)
    : m_Color(color)
{
    // Calculate center and store vertices in local space
    glm::vec3 center = (v0 + v1 + v2) / 3.0f;
    m_Vertices[0] = v0 - center;
    m_Vertices[1] = v1 - center;
    m_Vertices[2] = v2 - center;
}

TriangleRenderer::~TriangleRenderer()
{
    CleanUp();
}

void TriangleRenderer::Initialize(const std::shared_ptr<Shader>& shader)
{
    m_Shader = shader;
    
    // Create buffer with triangle vertices
    auto vertices = CreateVertices();
    
    // Setup the buffer with vertices
    m_Buffer.Setup(vertices);
}

void TriangleRenderer::Render(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    if (!m_Shader)
        return;
        
    m_Shader->Use();
    
    // Set matrices - Apply the model matrix transformation
    m_Shader->SetMat4("model", modelMatrix);
    m_Shader->SetMat4("view", viewMatrix);
    m_Shader->SetMat4("projection", projectionMatrix);
    
    // Material uniforms are now handled by the RenderSystem through UBOs
    // m_Shader->SetVec3("material.ambientColor", m_Material.m_AmbientColor);
    // m_Shader->SetFloat("material.ambientIntensity", m_Material.m_AmbientIntensity);
    // m_Shader->SetVec3("material.diffuseColor", m_Material.m_DiffuseColor);
    // m_Shader->SetFloat("material.diffuseIntensity", m_Material.m_DiffuseIntensity);
    // m_Shader->SetVec3("material.specularColor", m_Material.m_SpecularColor);
    // m_Shader->SetFloat("material.specularIntensity", m_Material.m_SpecularIntensity);
    // m_Shader->SetFloat("material.shininess", m_Material.m_Shininess);
    
    // Bind buffer
    m_Buffer.Bind();
    
    // Draw triangle (3 vertices)
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Buffer.GetVertexCount()));
    
    // Unbind
    m_Buffer.Unbind();
}

void TriangleRenderer::CleanUp()
{
    // Buffer destructor will handle cleanup
}

void TriangleRenderer::SetVertices(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
{
    // Calculate center and store vertices in local space
    glm::vec3 center = (v0 + v1 + v2) / 3.0f;
    m_Vertices[0] = v0 - center;
    m_Vertices[1] = v1 - center;
    m_Vertices[2] = v2 - center;
    
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

void TriangleRenderer::GetVertices(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2) const
{
    // Note: These are local space vertices. To get world space vertices,
    // you need to transform them using the entity's transform matrix.
    v0 = m_Vertices[0];
    v1 = m_Vertices[1];
    v2 = m_Vertices[2];
}

void TriangleRenderer::SetColor(const glm::vec3& color)
{
    m_Color = color;
    
    // Recreate vertices if buffer is setup
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

glm::vec3 TriangleRenderer::GetColor() const
{
    return m_Color;
}

std::vector<Vertex> TriangleRenderer::CreateVertices()
{
    std::vector<Vertex> vertices;
    
    // Compute triangle normal
    glm::vec3 normal = ComputeNormal();
    
    // Calculate UV coordinates (simple barycentric mapping)
    glm::vec2 uv0(0.0f, 0.0f);
    glm::vec2 uv1(1.0f, 0.0f);
    glm::vec2 uv2(0.5f, 1.0f);
    
    // Create vertices
    vertices.push_back({ m_Vertices[0], m_Color, normal, uv0 });
    vertices.push_back({ m_Vertices[1], m_Color, normal, uv1 });
    vertices.push_back({ m_Vertices[2], m_Color, normal, uv2 });
    
    return vertices;
}

glm::vec3 TriangleRenderer::ComputeNormal() const
{
    glm::vec3 edge1 = m_Vertices[1] - m_Vertices[0];
    glm::vec3 edge2 = m_Vertices[2] - m_Vertices[0];
    return glm::normalize(glm::cross(edge1, edge2));
} 