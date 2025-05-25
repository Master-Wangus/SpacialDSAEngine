#include "RayRenderer.hpp"
#include "Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

RayRenderer::RayRenderer(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& color, float length)
    : m_Origin(origin), m_Direction(glm::normalize(direction)), m_Color(color), m_Length(length), m_Thickness(0.05f)
{
}

RayRenderer::~RayRenderer()
{
    CleanUp();
}

void RayRenderer::Initialize(const std::shared_ptr<Shader>& shader)
{
    m_Shader = shader;
    
    // Create buffer with ray vertices
    auto vertices = CreateVertices();
    
    // Setup the buffer with vertices
    m_Buffer.Setup(vertices);
}

void RayRenderer::Render(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    if (!m_Shader)
        return;
        
    m_Shader->Use();
    
    // Set matrices
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
    
    // Bind buffer and draw
    m_Buffer.Bind();
    
    // Set line width for better visibility
    GLfloat oldLineWidth;
    glGetFloatv(GL_LINE_WIDTH, &oldLineWidth);
    glLineWidth(3.0f); // Make lines thicker for better visibility
    
    // Draw ray as a line
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_Buffer.GetVertexCount()));
    
    // Restore previous line width
    glLineWidth(oldLineWidth);
    
    m_Buffer.Unbind();
}

void RayRenderer::CleanUp()
{
    // Buffer destructor will handle cleanup
}

// Getters and setters
void RayRenderer::SetOrigin(const glm::vec3& origin)
{
    m_Origin = origin;
    
    // Recreate vertices if buffer is setup
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

glm::vec3 RayRenderer::GetOrigin() const
{
    return m_Origin;
}

void RayRenderer::SetDirection(const glm::vec3& direction)
{
    m_Direction = glm::normalize(direction);
    
    // Recreate vertices if buffer is setup
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

glm::vec3 RayRenderer::GetDirection() const
{
    return m_Direction;
}

void RayRenderer::SetColor(const glm::vec3& color)
{
    m_Color = color;
    
    // Recreate vertices if buffer is setup
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

glm::vec3 RayRenderer::GetColor() const
{
    return m_Color;
}

void RayRenderer::SetLength(float length)
{
    m_Length = length;
    
    // Recreate vertices if buffer is setup
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

float RayRenderer::GetLength() const
{
    return m_Length;
}

void RayRenderer::SetThickness(float thickness)
{
    m_Thickness = thickness;
    
    // Recreate vertices if buffer is setup
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

float RayRenderer::GetThickness() const
{
    return m_Thickness;
}

std::vector<Vertex> RayRenderer::CreateVertices()
{
    std::vector<Vertex> vertices;
    
    // Create just two vertices for the line: origin and endpoint
    glm::vec3 start = m_Origin;
    glm::vec3 end = m_Origin + m_Direction * m_Length;
    
    // Normal doesn't matter much for lines, but we'll set it to the direction
    glm::vec3 normal = m_Direction;
    
    // Create the line vertices
    vertices.push_back({ start, m_Color, normal, glm::vec2(0.0f, 0.0f) });
    vertices.push_back({ end, m_Color, normal, glm::vec2(1.0f, 1.0f) });
    
    return vertices;
} 