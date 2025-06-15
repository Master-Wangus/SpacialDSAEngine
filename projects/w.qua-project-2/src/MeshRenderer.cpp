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
        
        if (m_Wireframe)
        {
            // For wireframe, we need to create line vertices
            auto wireframeVertices = CreateWireframeVertices(meshVertices);
            
            // Apply color to wireframe vertices in-place
            for (auto& vertex : wireframeVertices)
            {
                vertex.m_Color = m_Color;
            }
            
            m_Buffer.Setup(wireframeVertices);
            m_WireframeVertexCount = static_cast<int>(wireframeVertices.size());
        }
        else
        {
            // For solid rendering, make a copy and apply color
            std::vector<Vertex> vertices;
            vertices.reserve(meshVertices.size()); // Pre-allocate
            
            // Copy and apply color in one pass
            for (const auto& vertex : meshVertices)
            {
                vertices.emplace_back(vertex.m_Position, m_Color, vertex.m_Normal, vertex.m_UV);
            }
            
            m_Buffer.Setup(vertices);
            m_WireframeVertexCount = 0;
        }
        
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
    
    m_Buffer.Bind();
    
    if (m_Wireframe)
    {
        glDrawArrays(GL_LINES, 0, m_WireframeVertexCount);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Buffer.GetVertexCount()));
    }
    
    m_Buffer.Unbind();
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
    if (m_Wireframe == wireframe)
        return;
        
    m_Wireframe = wireframe;
    
    if (m_Initialized && m_Shader)
    {
        m_Initialized = false;
        Initialize(m_Shader);
    }
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
    
    if (m_Wireframe)
    {
        // Convert triangles to lines for wireframe rendering
        auto wireframeVertices = CreateWireframeVertices(vertices);
        m_Buffer.Setup(wireframeVertices);
        m_WireframeVertexCount = static_cast<int>(wireframeVertices.size());
    }
    else
    {
        // Update our vertex buffer with the new colored vertices
        m_Buffer.Setup(vertices);
        m_WireframeVertexCount = 0;
    }
}

std::vector<Vertex> MeshRenderer::CreateWireframeVertices(const std::vector<Vertex>& triangleVertices)
{
    std::vector<Vertex> wireframeVertices;
    
    for (size_t i = 0; i < triangleVertices.size(); i += 3)
    {
        if (i + 2 < triangleVertices.size())
        {
            const Vertex& v0 = triangleVertices[i];
            const Vertex& v1 = triangleVertices[i + 1];
            const Vertex& v2 = triangleVertices[i + 2];
            
            // Line 1: v0 -> v1
            wireframeVertices.push_back(v0);
            wireframeVertices.push_back(v1);
            
            // Line 2: v1 -> v2
            wireframeVertices.push_back(v1);
            wireframeVertices.push_back(v2);
            
            // Line 3: v2 -> v0
            wireframeVertices.push_back(v2);
            wireframeVertices.push_back(v0);
        }
    }
    
    return wireframeVertices;
} 