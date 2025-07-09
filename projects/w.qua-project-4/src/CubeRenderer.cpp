/**
 * @class CubeRenderer
 * @brief Renderer for 3D cube primitives with custom color and transformation.
 *
 * This class implements the IRenderable interface to render 3D cubes using OpenGL.
 * It manages vertex data, buffers, and rendering operations with proper shading support.
 */

#include "CubeRenderer.hpp"
#include "Shader.hpp"

CubeRenderer::CubeRenderer(const glm::vec3& center, const glm::vec3& size, const glm::vec3& color)
    : m_Center(center), m_Size(size), m_Color(color), m_Wireframe(false), m_IsOriented(false)
{
}

CubeRenderer::CubeRenderer(const glm::vec3& center, const glm::vec3& size, const glm::vec3& color, bool wireframe)
    : m_Center(center), m_Size(size), m_Color(color), m_Wireframe(wireframe), m_IsOriented(false)
{
}

// Constructor for oriented bounding boxes
CubeRenderer::CubeRenderer(const glm::vec3& center, const glm::vec3 axes[3], const glm::vec3& halfExtents, const glm::vec3& color, bool wireframe)
    : m_Center(center), m_Size(halfExtents * 2.0f), m_Color(color), m_Wireframe(wireframe), m_IsOriented(true), m_HalfExtents(halfExtents)
{
    m_Axes[0] = axes[0];
    m_Axes[1] = axes[1];
    m_Axes[2] = axes[2];
}

CubeRenderer::~CubeRenderer()
{
    CleanUp();
}

void CubeRenderer::Initialize(const std::shared_ptr<Shader>& shader)
{
    m_Shader = shader;
    
    // Always create solid vertices - wireframe handled by glPolygonMode
    std::vector<Vertex> vertices = CreateVertices();
    m_Buffer.Setup(vertices);
}

void CubeRenderer::Render(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    if (!m_Shader)
        return;
        
    m_Shader->Use();
        
    m_Shader->SetMat4("model", modelMatrix);
    m_Shader->SetMat4("view", viewMatrix);
    m_Shader->SetMat4("projection", projectionMatrix);
    
    // Preserve the existing polygon mode so higher-level systems (like the global
    // wireframe toggle) remain in control after this renderer finishes.
    GLint prevPolygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, prevPolygonMode);

    if (m_Wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    // Bind buffer
    m_Buffer.Bind();
    
    // Always draw as triangles - glPolygonMode handles wireframe conversion
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Buffer.GetVertexCount()));
    
    // Unbind
    m_Buffer.Unbind();
    
    // Restore previous polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, prevPolygonMode[0]);
}

void CubeRenderer::CleanUp()
{
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
        std::vector<Vertex> vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

glm::vec3 CubeRenderer::GetColor() const
{
    return m_Color;
}

void CubeRenderer::SetWireframe(bool wireframe)
{
    m_Wireframe = wireframe;
    // No need to recreate vertices - wireframe handled by glPolygonMode
}

bool CubeRenderer::IsWireframe() const
{
    return m_Wireframe;
}

void CubeRenderer::SetOrientation(const glm::vec3 axes[3])
{
    m_Axes[0] = axes[0];
    m_Axes[1] = axes[1];
    m_Axes[2] = axes[2];
    m_IsOriented = true;
    
    // Recreate vertices if buffer is setup
    if (m_Buffer.GetVertexCount() > 0)
    {
        std::vector<Vertex> vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

void CubeRenderer::SetHalfExtents(const glm::vec3& halfExtents)
{
    m_HalfExtents = halfExtents;
    m_Size = halfExtents * 2.0f;
    
    // Recreate vertices if buffer is setup
    if (m_Buffer.GetVertexCount() > 0)
    {
        std::vector<Vertex> vertices = CreateVertices();
        m_Buffer.UpdateVertices(vertices);
    }
}

std::vector<Vertex> CubeRenderer::CreateVertices()
{
    std::vector<Vertex> vertices;

    glm::vec3 halfSize = m_Size * 0.5f;
    
    glm::vec3 pos[8];
    if (!m_IsOriented)
    {
        // Axis-aligned cube (AABB)
        pos[0] = m_Center + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z);
        pos[1] = m_Center + glm::vec3( halfSize.x, -halfSize.y, -halfSize.z);
        pos[2] = m_Center + glm::vec3( halfSize.x,  halfSize.y, -halfSize.z);
        pos[3] = m_Center + glm::vec3(-halfSize.x,  halfSize.y, -halfSize.z);
        pos[4] = m_Center + glm::vec3(-halfSize.x, -halfSize.y,  halfSize.z);
        pos[5] = m_Center + glm::vec3( halfSize.x, -halfSize.y,  halfSize.z);
        pos[6] = m_Center + glm::vec3( halfSize.x,  halfSize.y,  halfSize.z);
        pos[7] = m_Center + glm::vec3(-halfSize.x,  halfSize.y,  halfSize.z);
    }
    else
    {
        // Oriented cube (OBB) – build each corner using axes and half-extents
        glm::vec3 he = m_HalfExtents;
        // Precompute orientation vectors scaled by half extents
        glm::vec3 ax = m_Axes[0] * he.x;
        glm::vec3 ay = m_Axes[1] * he.y;
        glm::vec3 az = m_Axes[2] * he.z;

        // Construct the 8 corners
        pos[0] = m_Center - ax - ay - az;
        pos[1] = m_Center + ax - ay - az;
        pos[2] = m_Center + ax + ay - az;
        pos[3] = m_Center - ax + ay - az;
        pos[4] = m_Center - ax - ay + az;
        pos[5] = m_Center + ax - ay + az;
        pos[6] = m_Center + ax + ay + az;
        pos[7] = m_Center - ax + ay + az;
    }
    
    const glm::vec3 normals[6] = 
    {
        glm::vec3( 0.0f,  0.0f, -1.0f), // Back
        glm::vec3( 0.0f,  0.0f,  1.0f), // Front
        glm::vec3(-1.0f,  0.0f,  0.0f), // Left
        glm::vec3( 1.0f,  0.0f,  0.0f), // Right
        glm::vec3( 0.0f, -1.0f,  0.0f), // Bottom
        glm::vec3( 0.0f,  1.0f,  0.0f)  // Top
    };
    
    const int indices[36] = 
    {
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
    
    for (int i = 0; i < 36; i += 3) 
    {
        int faceIndex = i / 6;
        glm::vec3 normal = normals[faceIndex];
        
        Vertex v1 = { pos[indices[i]], m_Color, normal, glm::vec2(0.0f, 0.0f) };
        Vertex v2 = { pos[indices[i+1]], m_Color, normal, glm::vec2(1.0f, 0.0f) };
        Vertex v3 = { pos[indices[i+2]], m_Color, normal, glm::vec2(0.5f, 1.0f) };
        
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
    }
    
    return vertices;
}

 