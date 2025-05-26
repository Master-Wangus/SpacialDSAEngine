/**
 * @class TriangleRenderer
 * @brief Renderer for 3D triangle primitives with vertex color support.
 *
 * This class implements the IRenderable interface to render 3D triangles using OpenGL.
 * It manages triangle vertex data, normal calculation, and provides efficient rendering capabilities.
 */

#pragma once

#include "pch.h"
#include "Buffer.hpp"
#include "Lighting.hpp"
#include "IRenderable.hpp"

class Shader;

class TriangleRenderer : public IRenderable
{
public:
    TriangleRenderer(const glm::vec3& v0 = glm::vec3(0.0f, 0.0f, 0.0f),
                    const glm::vec3& v1 = glm::vec3(1.0f, 0.0f, 0.0f),
                    const glm::vec3& v2 = glm::vec3(0.0f, 1.0f, 0.0f),
                    const glm::vec3& color = glm::vec3(0.0f, 0.8f, 0.0f));
    
    ~TriangleRenderer();
    
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    void Render(const glm::mat4& modelMatrix, 
               const glm::mat4& viewMatrix, 
               const glm::mat4& projectionMatrix) override;
    
    void CleanUp() override;
    
    // Triangle-specific functionality
    void SetVertices(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);
    void GetVertices(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2) const;
    void SetColor(const glm::vec3& color);
    glm::vec3 GetColor() const;
    
private:
    glm::vec3 m_Vertices[3];
    glm::vec3 m_Color;

    std::vector<Vertex> CreateVertices();
    glm::vec3 ComputeNormal() const;
}; 