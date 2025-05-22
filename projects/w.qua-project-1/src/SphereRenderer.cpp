#include "SphereRenderer.hpp"
#include "Shader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

SphereRenderer::SphereRenderer(const glm::vec3& center, float radius, const glm::vec3& color)
    : m_Center(center), m_Radius(radius), m_Color(color)
{
}

SphereRenderer::~SphereRenderer()
{
    CleanUp();
}

void SphereRenderer::Initialize(const std::shared_ptr<Shader>& shader)
{
    m_Shader = shader;
    
    // Create buffer with sphere vertices
    auto vertices = CreateVertices();
    
    // Setup the buffer with vertices
    m_Buffer.Setup(vertices);
}

void SphereRenderer::Render(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    if (!m_Shader)
        return;
        
    m_Shader->Use();
    
    m_Shader->SetMat4("model", modelMatrix);
    m_Shader->SetMat4("view", viewMatrix);
    m_Shader->SetMat4("projection", projectionMatrix);
    
    m_Shader->SetVec3("material.ambientColor", m_Material.m_AmbientColor);
    m_Shader->SetFloat("material.ambientIntensity", m_Material.m_AmbientIntensity);
    m_Shader->SetVec3("material.diffuseColor", m_Material.m_DiffuseColor);
    m_Shader->SetFloat("material.diffuseIntensity", m_Material.m_DiffuseIntensity);
    m_Shader->SetVec3("material.specularColor", m_Material.m_SpecularColor);
    m_Shader->SetFloat("material.specularIntensity", m_Material.m_SpecularIntensity);
    m_Shader->SetFloat("material.shininess", m_Material.m_Shininess);
    
    m_Buffer.Bind();
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Buffer.GetVertexCount()));
    
    m_Buffer.Unbind();
}

void SphereRenderer::CleanUp()
{

}

void SphereRenderer::SetCenter(const glm::vec3& center)
{
    m_Center = center;
}

glm::vec3 SphereRenderer::GetCenter() const
{
    return m_Center;
}

void SphereRenderer::SetRadius(float radius)
{
    m_Radius = radius;
}

float SphereRenderer::GetRadius() const
{
    return m_Radius;
}

void SphereRenderer::SetColor(const glm::vec3& color)
{
    m_Color = color;
    
    // Recreate vertices if buffer is setup
    if (m_Buffer.GetVertexCount() > 0)
    {
        auto vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

glm::vec3 SphereRenderer::GetColor() const
{
    return m_Color;
}

std::vector<Vertex> SphereRenderer::CreateVertices()
{
    std::vector<Vertex> vertices;
    
    // Create a unit sphere (radius 1.0) - scaling will be applied in the model matrix
    const int rings = 16;
    const int sectors = 16;
    
    float const R = 1.0f / (float)(rings - 1);
    float const S = 1.0f / (float)(sectors - 1);
    
    // Generate vertices and normals for a sphere
    std::vector<glm::vec3> sphereVertices;
    std::vector<glm::vec3> sphereNormals;
    std::vector<glm::vec2> sphereUVs;
    
    for (int r = 0; r < rings; ++r)
    {
        for (int s = 0; s < sectors; ++s)
        {
            float const y = sin(-glm::half_pi<float>() + glm::pi<float>() * r * R);
            float const x = cos(2 * glm::pi<float>() * s * S) * sin(glm::pi<float>() * r * R);
            float const z = sin(2 * glm::pi<float>() * s * S) * sin(glm::pi<float>() * r * R);
            
            // UV coordinates
            float u = static_cast<float>(s) / (sectors - 1);
            float v = static_cast<float>(r) / (rings - 1);
            
            // Unit sphere with radius 1.0
            sphereVertices.push_back(glm::vec3(x, y, z));
            // Normal is same as position for a unit sphere at origin
            sphereNormals.push_back(glm::normalize(glm::vec3(x, y, z)));
            // UV coordinates
            sphereUVs.push_back(glm::vec2(u, v));
        }
    }
    
    // Generate triangle indices
    for (int r = 0; r < rings - 1; ++r)
    {
        for (int s = 0; s < sectors - 1; ++s)
        {
            // Current vertex and the three around it form two triangles
            int current = r * sectors + s;
            int next = current + 1;
            int nextRing = current + sectors;
            int nextRingNext = nextRing + 1;
            
            // First triangle
            vertices.push_back({ sphereVertices[current], m_Color, sphereNormals[current], sphereUVs[current] });
            vertices.push_back({ sphereVertices[nextRing], m_Color, sphereNormals[nextRing], sphereUVs[nextRing] });
            vertices.push_back({ sphereVertices[next], m_Color, sphereNormals[next], sphereUVs[next] });
            
            // Second triangle
            vertices.push_back({ sphereVertices[next], m_Color, sphereNormals[next], sphereUVs[next] });
            vertices.push_back({ sphereVertices[nextRing], m_Color, sphereNormals[nextRing], sphereUVs[nextRing] });
            vertices.push_back({ sphereVertices[nextRingNext], m_Color, sphereNormals[nextRingNext], sphereUVs[nextRingNext] });
        }
    }
    
    return vertices;
} 