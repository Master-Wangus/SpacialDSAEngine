#include "PlaneRenderer.hpp"
#include "Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>

PlaneRenderer::PlaneRenderer(const glm::vec3& normal, float distance, const glm::vec3& color, float size)
    : m_Normal(normal), m_Distance(distance), m_Color(color), m_Size(size)
{
}

PlaneRenderer::~PlaneRenderer()
{
    CleanUp();
}

void PlaneRenderer::Initialize(const std::shared_ptr<Shader>& shader)
{
    m_Shader = shader;
    auto vertices = CreateVertices();
    m_Buffer.Setup(vertices);
}

void PlaneRenderer::Render(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    if (!m_Shader)
        return;
        
    m_Shader->Use();
    
    // Set matrices
    m_Shader->SetMat4("model", modelMatrix);
    m_Shader->SetMat4("view", viewMatrix);
    m_Shader->SetMat4("projection", projectionMatrix);
   
    
    // Bind buffer and draw
    m_Buffer.Bind();
    
    // Draw triangles (solid rendering)
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Buffer.GetVertexCount()));
    
    m_Buffer.Unbind();
}

void PlaneRenderer::CleanUp()
{
    // Buffer destructor will handle cleanup
}

void PlaneRenderer::SetNormal(const glm::vec3& normal)
{
    m_Normal = normal;
    
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

glm::vec3 PlaneRenderer::GetNormal() const
{
    return m_Normal;
}

void PlaneRenderer::SetDistance(float distance)
{
    m_Distance = distance;
    
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

float PlaneRenderer::GetDistance() const
{
    return m_Distance;
}

void PlaneRenderer::SetColor(const glm::vec3& color)
{
    m_Color = color;
    
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

glm::vec3 PlaneRenderer::GetColor() const
{
    return m_Color;
}

void PlaneRenderer::SetSize(float size)
{
    m_Size = size;
    
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

float PlaneRenderer::GetSize() const
{
    return m_Size;
}

std::vector<Vertex> PlaneRenderer::CreateVertices()
{
    std::vector<Vertex> vertices;
    
    glm::vec3 normal = glm::normalize(m_Normal);
    
    // Create a coordinate system for the plane
    glm::vec3 tangent, bitangent;
    
    // Find tangent perpendicular to normal
    if (std::abs(normal.x) < std::abs(normal.y))
    {
        tangent = glm::normalize(glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), normal));
    }
    else
    {
        tangent = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), normal));
    }
    
    // Find bitangent perpendicular to normal and tangent
    bitangent = glm::normalize(glm::cross(normal, tangent));
    
    // Find a point on the plane
    glm::vec3 planePoint = normal * m_Distance;
    
    // Calculate half size
    float halfSize = m_Size * 0.5f;
    
    // Calculate the four corners of the quad
    glm::vec3 corner1 = planePoint - tangent * halfSize - bitangent * halfSize;
    glm::vec3 corner2 = planePoint + tangent * halfSize - bitangent * halfSize;
    glm::vec3 corner3 = planePoint + tangent * halfSize + bitangent * halfSize;
    glm::vec3 corner4 = planePoint - tangent * halfSize + bitangent * halfSize;
    
    // UV coordinates
    glm::vec2 uv1(0.0f, 0.0f);
    glm::vec2 uv2(1.0f, 0.0f);
    glm::vec2 uv3(1.0f, 1.0f);
    glm::vec2 uv4(0.0f, 1.0f);
    
    // First triangle
    vertices.push_back({ corner1, m_Color, normal, uv1 });
    vertices.push_back({ corner2, m_Color, normal, uv2 });
    vertices.push_back({ corner3, m_Color, normal, uv3 });
    
    // Second triangle
    vertices.push_back({ corner1, m_Color, normal, uv1 });
    vertices.push_back({ corner3, m_Color, normal, uv3 });
    vertices.push_back({ corner4, m_Color, normal, uv4 });
    
    return vertices;
} 