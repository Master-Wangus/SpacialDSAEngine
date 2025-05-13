#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

class IRenderable 
{
public:

    virtual ~IRenderable() = default;
    virtual void initialize() = 0;
    virtual void render(const glm::mat4& modelMatrix, 
                       const glm::mat4& viewMatrix, 
                       const glm::mat4& projectionMatrix) = 0;
    virtual void cleanUp() = 0;
}; 