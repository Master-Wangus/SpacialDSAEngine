#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Buffer.hpp"
#include "Lighting.hpp"
#include <memory>

class Shader;

class IRenderable 
{
public:
    virtual ~IRenderable() = default;
    virtual void Initialize(const std::shared_ptr<Shader>& shader) = 0;
    virtual void Render(const glm::mat4& modelMatrix, 
                       const glm::mat4& viewMatrix, 
                       const glm::mat4& projectionMatrix) = 0;
    virtual void CleanUp() = 0;
    
    // Material methods
    virtual void SetMaterial(const Material& material) { m_Material = material; }
    virtual Material GetMaterial() const { return m_Material; }

protected:
    Buffer m_buffer;
    std::shared_ptr<Shader> m_shader;
    glm::mat4 m_ModelMatrix;
    Material m_Material; // Default material (white)
}; 