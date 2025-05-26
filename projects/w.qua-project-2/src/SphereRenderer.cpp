/**
 * @class SphereRenderer
 * @brief Renderer for 3D sphere primitives with customizable properties.
 *
 * This class implements the IRenderable interface to render 3D spheres using OpenGL.
 * It manages vertex generation for sphere approximation, buffer handling, and rendering with proper shading.
 */

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
    
    auto vertices = CreateVertices();
    
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
    
    // Parameters for sphere generation
    float radius = 1.0f; // Unit sphere - scaling applied via model matrix
    int sectors = 36;    // Horizontal divisions (longitude)
    int stacks = 18;     // Vertical divisions (latitude)
    
    // Generate vertices and normals for a sphere
    std::vector<glm::vec3> sphereVertices;
    std::vector<glm::vec3> sphereNormals;
    std::vector<glm::vec2> sphereUVs;
    
    // Generate sphere points using latitude/longitude approach
    for(int i = 0; i <= stacks; ++i) 
    {
        float V = i / (float)stacks;
        float phi = V * glm::pi<float>(); // Latitude angle from 0 to PI
        
        for(int j = 0; j <= sectors; ++j) 
        {
            float U = j / (float)sectors;
            float theta = U * (glm::pi<float>() * 2); // Longitude angle from 0 to 2*PI
            
            // Convert spherical to Cartesian coordinates
            float x = cosf(theta) * sinf(phi);
            float y = cosf(phi);
            float z = sinf(theta) * sinf(phi);
            
            // Unit sphere with radius 1.0
            glm::vec3 position = glm::vec3(x, y, z);
            sphereVertices.push_back(position);
            // Normal is same as position for a unit sphere at origin
            sphereNormals.push_back(glm::normalize(position));
            // UV coordinates
            sphereUVs.push_back(glm::vec2(U, V));
        }
    }
    
    // Generate triangle indices
    for(int i = 0; i < stacks; ++i) 
    {
        for(int j = 0; j < sectors; ++j) 
        {
            // Calculate the four corner indices
            int current = i * (sectors + 1) + j;
            int next = current + 1;
            int nextStack = (i + 1) * (sectors + 1) + j;
            int nextStackNext = nextStack + 1;
            
            // Skip degenerate triangles at the poles
            if(i != 0) 
            {
                // Upper triangle
                vertices.push_back({ sphereVertices[current], m_Color, sphereNormals[current], sphereUVs[current] });
                vertices.push_back({ sphereVertices[nextStack], m_Color, sphereNormals[nextStack], sphereUVs[nextStack] });
                vertices.push_back({ sphereVertices[next], m_Color, sphereNormals[next], sphereUVs[next] });
            }
            
            if(i != (stacks-1)) 
            {
                // Lower triangle
                vertices.push_back({ sphereVertices[next], m_Color, sphereNormals[next], sphereUVs[next] });
                vertices.push_back({ sphereVertices[nextStack], m_Color, sphereNormals[nextStack], sphereUVs[nextStack] });
                vertices.push_back({ sphereVertices[nextStackNext], m_Color, sphereNormals[nextStackNext], sphereUVs[nextStackNext] });
            }
        }
    }
    
    return vertices;
} 