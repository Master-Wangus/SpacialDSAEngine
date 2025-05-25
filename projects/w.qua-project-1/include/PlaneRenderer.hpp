#pragma once

#include "pch.h"
#include "Buffer.hpp"
#include "IRenderable.hpp"

class Shader;

class PlaneRenderer : public IRenderable
{
public:
    PlaneRenderer(const glm::vec3& normal = glm::vec3(0.0f, 1.0f, 0.0f), 
                 float distance = 0.0f,
                 const glm::vec3& color = glm::vec3(0.8f, 0.8f, 0.8f), 
                 float size = 10.0f);
    
    ~PlaneRenderer();
    
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    void Render(const glm::mat4& modelMatrix, 
               const glm::mat4& viewMatrix, 
               const glm::mat4& projectionMatrix) override;
    
    void CleanUp() override;
    
    // Plane-specific functionality
    void SetNormal(const glm::vec3& normal);
    glm::vec3 GetNormal() const;
    void SetDistance(float distance);
    float GetDistance() const;
    void SetColor(const glm::vec3& color);
    glm::vec3 GetColor() const;
    void SetSize(float size);
    float GetSize() const;
    
private:
    glm::vec3 m_Normal;
    float m_Distance;
    float m_Size;
    glm::vec3 m_Color;

    std::vector<Vertex> CreateVertices();
}; 