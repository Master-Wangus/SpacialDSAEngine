/**
 * @file MeshRenderer.cpp
 * @brief Implementation of the MeshRenderer class for rendering mesh resources.
 */

#include "MeshRenderer.hpp"
#include "ResourceSystem.hpp"
#include "Shader.hpp"

MeshRenderer::MeshRenderer(const ResourceHandle& meshHandle)
    : m_MeshHandle(meshHandle), m_Color(1.0f, 1.0f, 1.0f), m_Wireframe(false)
{
}

MeshRenderer::MeshRenderer(const ResourceHandle& meshHandle, const glm::vec3& color)
    : m_MeshHandle(meshHandle), m_Color(color), m_Wireframe(false)
{
}

MeshRenderer::MeshRenderer(const ResourceHandle& meshHandle, const glm::vec3& color, bool wireframe)
    : m_MeshHandle(meshHandle), m_Color(color), m_Wireframe(wireframe)
{
}

MeshRenderer::~MeshRenderer()
{
    CleanUp();
}

void MeshRenderer::Initialize(const std::shared_ptr<Shader>& shader)
{
    m_Shader = shader;
    
    // Get the mesh from the resource system
    auto mesh = ResourceSystem::GetInstance().GetMesh(m_MeshHandle);
    
    if (mesh)
    {
        // Get vertices and reserve space to avoid reallocations
        const auto& meshVertices = mesh->GetVertexes();
        
        // Always use the same vertices - wireframe handled by glPolygonMode
        std::vector<Vertex> vertices;
        vertices.reserve(meshVertices.size()); // Pre-allocate
        
        // Copy and apply color in one pass
        for (const auto& vertex : meshVertices)
        {
            vertices.emplace_back(vertex.m_Position, m_Color, vertex.m_Normal, vertex.m_UV);
        }
        
        m_Buffer.Setup(vertices);
        m_Initialized = true;
    }
    else
    {
        std::cerr << "Failed to initialize MeshRenderer: Invalid mesh handle" << std::endl;
    }
}

void MeshRenderer::Render(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
    if (!m_Initialized || !m_Shader)
        return;
    
    m_Shader->Use();
    
    m_Shader->SetMat4("model", modelMatrix);
    m_Shader->SetMat4("view", viewMatrix);
    m_Shader->SetMat4("projection", projectionMatrix);
    
    // Save current polygon mode so we can restore it after rendering. This allows
    // higher-level systems (e.g. a global wireframe toggle) to control the mode
    // without being overridden by individual renderers.
    GLint prevPolygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, prevPolygonMode);

    // Apply local wireframe setting *only* if this renderer explicitly requests
    // wireframe drawing. Otherwise, keep the mode that was already active.
    if (m_Wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    
    m_Buffer.Bind();
    
    // Always draw as triangles - glPolygonMode handles wireframe conversion
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Buffer.GetVertexCount()));
    
    m_Buffer.Unbind();
    
    // Restore previous polygon mode so subsequent renderers keep the expected state
    glPolygonMode(GL_FRONT_AND_BACK, prevPolygonMode[0]);
}

void MeshRenderer::CleanUp()
{
    m_Initialized = false;
}

void MeshRenderer::SetMesh(const ResourceHandle& meshHandle)
{
    if (m_MeshHandle == meshHandle)
        return;
    
    m_MeshHandle = meshHandle;
    
    if (m_Initialized && m_Shader)
    {
        m_Initialized = false;
        Initialize(m_Shader);
    }
}

void MeshRenderer::SetColor(const glm::vec3& color)
{
    m_Color = color;
    
    if (m_Initialized)
    {
        UpdateVertexColors();
    }
}

glm::vec3 MeshRenderer::GetColor() const
{
    return m_Color;
}

void MeshRenderer::SetWireframe(bool wireframe)
{
    m_Wireframe = wireframe;
    // No need to recreate vertices - wireframe handled by glPolygonMode
}

bool MeshRenderer::IsWireframe() const
{
    return m_Wireframe;
}

void MeshRenderer::UpdateVertexColors()
{
    if (!m_Initialized || !m_Shader)
        return;
    
    auto mesh = ResourceSystem::GetInstance().GetMesh(m_MeshHandle);
    if (!mesh)
        return;
    
    auto vertices = mesh->GetVertexes();
    
    for (auto& vertex : vertices)
    {
        vertex.m_Color = m_Color;
    }
    
    // Update our vertex buffer with the new colored vertices
    m_Buffer.Setup(vertices);
}

 