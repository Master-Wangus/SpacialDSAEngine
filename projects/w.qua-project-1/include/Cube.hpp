#pragma once

#include "pch.h"
#include "Buffer.hpp"
#include "Lighting.hpp"
#include "Primitives.hpp"
#include "IRenderable.hpp"

class Shader;

class Cube : public IRenderable
{
public:
    Cube(const glm::vec3& color = glm::vec3(1.0f), float size = 1.0f);
    
    ~Cube();
    
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    void Render(const glm::mat4& modelMatrix, 
               const glm::mat4& viewMatrix, 
               const glm::mat4& projectionMatrix) override;
    
    void CleanUp() override;
    
    // Mesh-specific functionality
    void SetColor(const glm::vec3& color);
    glm::vec3 GetColor() const;
    float GetSize() const;
    
private:
    float m_Size;
    glm::vec3 m_Color;

    std::vector<Vertex> CreateVertices();
};