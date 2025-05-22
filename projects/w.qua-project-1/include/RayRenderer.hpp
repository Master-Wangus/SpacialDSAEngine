#pragma once

#include "pch.h"
#include "Buffer.hpp"
#include "Lighting.hpp"
#include "IRenderable.hpp"

class Shader;

class RayRenderer : public IRenderable
{
public:
    RayRenderer(const glm::vec3& origin = glm::vec3(0.0f), 
               const glm::vec3& direction = glm::vec3(0.0f, 0.0f, 1.0f),
               const glm::vec3& color = glm::vec3(1.0f, 1.0f, 0.0f),
               float length = 10.0f);
    
    ~RayRenderer();
    
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    void Render(const glm::mat4& modelMatrix, 
               const glm::mat4& viewMatrix, 
               const glm::mat4& projectionMatrix) override;
    
    void CleanUp() override;
    
    // Ray-specific functionality
    void SetOrigin(const glm::vec3& origin);
    glm::vec3 GetOrigin() const;
    void SetDirection(const glm::vec3& direction);
    glm::vec3 GetDirection() const;
    void SetColor(const glm::vec3& color);
    glm::vec3 GetColor() const;
    void SetLength(float length);
    float GetLength() const;
    void SetThickness(float thickness);
    float GetThickness() const;
    
private:
    glm::vec3 m_Origin;
    glm::vec3 m_Direction;
    glm::vec3 m_Color;
    float m_Length;
    float m_Thickness;

    std::vector<Vertex> CreateVertices();
}; 