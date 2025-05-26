/**
 * @file MeshRenderer.cpp
 * @brief Implementation of the MeshRenderer class for rendering mesh resources.
 */

#include "MeshRenderer.hpp"
#include "ResourceSystem.hpp"
#include "Shader.hpp"

MeshRenderer::MeshRenderer(const ResourceHandle& meshHandle)
    : m_MeshHandle(meshHandle), m_Color(1.0f, 1.0f, 1.0f)
{
}

MeshRenderer::MeshRenderer(const ResourceHandle& meshHandle, const glm::vec3& color)
    : m_MeshHandle(meshHandle), m_Color(color)
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
        // Make a copy of the mesh vertices with our custom color
        auto vertices = mesh->GetVertices();
        
        // Apply the color to all vertices
        for (auto& vertex : vertices)
        {
            vertex.m_Color = m_Color;
        }
        
        // Initialize buffer with our colored vertices
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
    
    // Use the shader
    m_Shader->Use();
    
    // Set matrices
    m_Shader->SetMat4("model", modelMatrix);
    m_Shader->SetMat4("view", viewMatrix);
    m_Shader->SetMat4("projection", projectionMatrix);
    
    // Bind our vertex buffer
    m_Buffer.Bind();
    
    // Draw the mesh
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Buffer.GetVertexCount()));
    
    // Unbind
    m_Buffer.Unbind();
}

void MeshRenderer::CleanUp()
{
    m_Initialized = false;
    // Buffer destructor will handle cleanup
}

void MeshRenderer::SetMesh(const ResourceHandle& meshHandle)
{
    if (m_MeshHandle == meshHandle)
        return;
    
    m_MeshHandle = meshHandle;
    
    // If already initialized, re-initialize with the new mesh
    if (m_Initialized && m_Shader)
    {
        m_Initialized = false;
        Initialize(m_Shader);
    }
}

void MeshRenderer::SetColor(const glm::vec3& color)
{
    m_Color = color;
    
    // If already initialized, update the vertex colors
    if (m_Initialized)
    {
        UpdateVertexColors();
    }
}

glm::vec3 MeshRenderer::GetColor() const
{
    return m_Color;
}

void MeshRenderer::UpdateVertexColors()
{
    if (!m_Initialized || !m_Shader)
        return;
    
    auto mesh = ResourceSystem::GetInstance().GetMesh(m_MeshHandle);
    if (!mesh)
        return;
    
    // Get original vertices from the mesh resource
    auto vertices = mesh->GetVertices();
    
    // Apply our color to all vertices
    for (auto& vertex : vertices)
    {
        vertex.m_Color = m_Color;
    }
    
    // Update our vertex buffer with the new colored vertices
    m_Buffer.Setup(vertices);
} 