#include "RayRenderer.hpp"
#include "Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

RayRenderer::RayRenderer(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& color, float length)
    : m_Origin(origin), m_Direction(glm::normalize(direction)), m_Color(color), m_Length(length), m_Thickness(0.02f)
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
    
    // We need a rotation matrix that aligns our local Z-axis with the direction vector
    glm::quat rotQuat = glm::rotation(glm::vec3(0.0f, 0.0f, 1.0f), m_Direction);
    glm::mat4 rotMatrix = glm::mat4_cast(rotQuat);
    
    // Combine the model matrix with our rotation, keeping position from model matrix
    glm::mat4 rayModelMatrix = modelMatrix * rotMatrix;
    
    // Set matrices
    m_Shader->SetMat4("model", rayModelMatrix);
    m_Shader->SetMat4("view", viewMatrix);
    m_Shader->SetMat4("projection", projectionMatrix);
    
    // Set material properties
    m_Shader->SetVec3("material.ambientColor", m_Material.m_AmbientColor);
    m_Shader->SetFloat("material.ambientIntensity", m_Material.m_AmbientIntensity);
    m_Shader->SetVec3("material.diffuseColor", m_Material.m_DiffuseColor);
    m_Shader->SetFloat("material.diffuseIntensity", m_Material.m_DiffuseIntensity);
    m_Shader->SetVec3("material.specularColor", m_Material.m_SpecularColor);
    m_Shader->SetFloat("material.specularIntensity", m_Material.m_SpecularIntensity);
    m_Shader->SetFloat("material.shininess", m_Material.m_Shininess);
    
    // Bind buffer and draw
    m_Buffer.Bind();
    
    // Draw ray as triangles (solid rendering)
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Buffer.GetVertexCount()));
    
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
    
    // For the ray, we'll create it from (0,0,0) to (0,0,length)
    // The model matrix will handle positioning and orientation
    glm::vec3 localOrigin = glm::vec3(0.0f);
    glm::vec3 localEndpoint = glm::vec3(0.0f, 0.0f, m_Length);
    
    // Create an orthonormal basis for the ray
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f); // Local forward direction
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);   // Local right direction
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);      // Local up direction
    
    // Create a thin cylinder representing the ray
    const int segments = 6;
    const float theta = glm::two_pi<float>() / segments;
    
    // Create end points around the ray
    std::vector<glm::vec3> originPoints;
    std::vector<glm::vec3> endPoints;
    std::vector<glm::vec2> texCoords;
    
    for (int i = 0; i < segments; ++i)
    {
        float angle = theta * i;
        float cosAngle = cos(angle);
        float sinAngle = sin(angle);
        
        glm::vec3 offset = right * cosAngle * m_Thickness + up * sinAngle * m_Thickness;
        
        originPoints.push_back(localOrigin + offset);
        endPoints.push_back(localEndpoint + offset);
        
        // Generate texture coordinates
        float u = static_cast<float>(i) / (segments - 1);
        texCoords.push_back(glm::vec2(u, 0.0f));  // Origin end
        texCoords.push_back(glm::vec2(u, 1.0f));  // Endpoint end
    }
    
    // Create triangles for the cylinder sides
    for (int i = 0; i < segments; ++i)
    {
        int nextIndex = (i + 1) % segments;
        
        // Calculate normal
        glm::vec3 tangent = endPoints[i] - originPoints[i];
        glm::vec3 binormal = originPoints[nextIndex] - originPoints[i];
        glm::vec3 normal = glm::normalize(glm::cross(tangent, binormal));
        
        // UV coordinates for this segment
        glm::vec2 uv1 = texCoords[i*2];         // Current origin point
        glm::vec2 uv2 = texCoords[nextIndex*2]; // Next origin point
        glm::vec2 uv3 = texCoords[i*2+1];       // Current end point
        glm::vec2 uv4 = texCoords[nextIndex*2+1]; // Next end point
        
        // First triangle
        vertices.push_back({ originPoints[i], m_Color, normal, uv1 });
        vertices.push_back({ originPoints[nextIndex], m_Color, normal, uv2 });
        vertices.push_back({ endPoints[i], m_Color, normal, uv3 });
        
        // Second triangle
        vertices.push_back({ originPoints[nextIndex], m_Color, normal, uv2 });
        vertices.push_back({ endPoints[nextIndex], m_Color, normal, uv4 });
        vertices.push_back({ endPoints[i], m_Color, normal, uv3 });
    }
    
    return vertices;
} 