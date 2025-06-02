/**
 * @class CubeRenderer
 * @brief Renderer for 3D cube primitives with custom color and transformation.
 *
 * This class implements the IRenderable interface to render 3D cubes using OpenGL.
 * It manages vertex data, buffers, and rendering operations with proper shading support.
 */

#pragma once

#include "pch.h"
#include "IRenderable.hpp"

class Shader;
struct Vertex;

class CubeRenderer : public IRenderable
{
public:
    CubeRenderer(const glm::vec3& center = glm::vec3(0.0f),
                const glm::vec3& size = glm::vec3(1.0f),
                const glm::vec3& color = glm::vec3(1.0f));
    
    CubeRenderer(const glm::vec3& center,
                const glm::vec3& size,
                const glm::vec3& color,
                bool wireframe);
    
    ~CubeRenderer();
    
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    void Render(const glm::mat4& modelMatrix, 
               const glm::mat4& viewMatrix, 
               const glm::mat4& projectionMatrix) override;
    
    void CleanUp() override;
    
    // Cube-specific functionality
    void SetCenter(const glm::vec3& center);
    glm::vec3 GetCenter() const;
    void SetSize(const glm::vec3& size);
    glm::vec3 GetSize() const;
    void SetColor(const glm::vec3& color);
    glm::vec3 GetColor() const;
    
    // Wireframe control
    void SetWireframe(bool wireframe);
    bool IsWireframe() const;
    
private:
    glm::vec3 m_Center;
    glm::vec3 m_Size;
    glm::vec3 m_Color;
    bool m_Wireframe = false;

    std::vector<Vertex> CreateVertices();
    std::vector<Vertex> CreateWireframeVertices();
}; 