#pragma once

#include "pch.h"
#include "Buffer.hpp"
#include "Lighting.hpp"
#include "IRenderable.hpp"

class Shader;

class CubeRenderer : public IRenderable
{
public:
    CubeRenderer(const glm::vec3& center = glm::vec3(0.0f),
                const glm::vec3& size = glm::vec3(1.0f),
                const glm::vec3& color = glm::vec3(1.0f));
    
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
    
private:
    glm::vec3 m_Center;
    glm::vec3 m_Size;
    glm::vec3 m_Color;

    std::vector<Vertex> CreateVertices();
}; 