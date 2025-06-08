/**
 * @class FrustumRenderer
 * @brief Renderer for view frustum visualization with wireframe display.
 *
 * This class implements the IRenderable interface to render view frustums using OpenGL.
 * It creates wireframe geometry for the 8 vertices and 12 edges of the frustum.
 */

#pragma once

#include "pch.h"
#include "IRenderable.hpp"

class Shader;
struct Vertex;

class FrustumRenderer : public IRenderable
{
public:
    FrustumRenderer(const glm::vec3& color = glm::vec3(1.0f, 0.0f, 1.0f)); // Default magenta
    ~FrustumRenderer();
    
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    void Render(const glm::mat4& modelMatrix, 
               const glm::mat4& viewMatrix, 
               const glm::mat4& projectionMatrix) override;
    
    void CleanUp() override;
    
    // Update frustum geometry based on camera parameters
    void UpdateFrustum(const glm::mat4& invViewProjection);
    void SetColor(const glm::vec3& color);
    glm::vec3 GetColor() const;
    
private:
    glm::vec3 m_Color;
    glm::mat4 m_InvViewProjection;
    bool m_FrustumDirty = true;

    std::vector<Vertex> CreateFrustumVertices();
    void UpdateVertices();
}; 