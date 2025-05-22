#pragma once

#include "pch.h"
#include "Buffer.hpp"
#include "Lighting.hpp"
#include "IRenderable.hpp"

class Shader;

class SphereRenderer : public IRenderable
{
public:
    SphereRenderer(const glm::vec3& center = glm::vec3(0.0f), 
                  float radius = 1.0f,
                  const glm::vec3& color = glm::vec3(1.0f, 0.0f, 0.0f));
    
    ~SphereRenderer();
    
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    void Render(const glm::mat4& modelMatrix, 
               const glm::mat4& viewMatrix, 
               const glm::mat4& projectionMatrix) override;
    
    void CleanUp() override;
    
    // Sphere-specific functionality
    void SetCenter(const glm::vec3& center);
    glm::vec3 GetCenter() const;
    void SetRadius(float radius);
    float GetRadius() const;
    void SetColor(const glm::vec3& color);
    glm::vec3 GetColor() const;
    
private:
    glm::vec3 m_Center;
    float m_Radius;
    glm::vec3 m_Color;

    std::vector<Vertex> CreateVertices();
}; 