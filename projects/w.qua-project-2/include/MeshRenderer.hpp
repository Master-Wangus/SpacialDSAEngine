/**
 * @file MeshRenderer.hpp
 * @brief Renderer for mesh resources with custom transformation.
 *
 * This class implements the IRenderable interface to render 3D meshes loaded as resources.
 */

#pragma once

#include "pch.h"
#include "IRenderable.hpp"
#include "ResourceSystem.hpp"

class MeshRenderer : public IRenderable 
{
public:
    explicit MeshRenderer(const ResourceHandle& meshHandle);
    MeshRenderer(const ResourceHandle& meshHandle, const glm::vec3& color);
    MeshRenderer(const ResourceHandle& meshHandle, const glm::vec3& color, bool wireframe);
    ~MeshRenderer() override;
    
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    void Render(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;
    void CleanUp() override;
    
    void SetMesh(const ResourceHandle& meshHandle);
    void SetColor(const glm::vec3& color);
    glm::vec3 GetColor() const;
    
    void SetWireframe(bool wireframe);
    bool IsWireframe() const;
    
private:
    ResourceHandle m_MeshHandle;
    glm::vec3 m_Color = glm::vec3(1.0f);
    bool m_Initialized = false;
    bool m_Wireframe = false;
    int m_WireframeVertexCount = 0;
    
    void UpdateVertexColors();
    std::vector<Vertex> CreateWireframeVertices(const std::vector<Vertex>& triangleVertices);
}; 