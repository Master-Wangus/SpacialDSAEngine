/**
 * @class IRenderable
 * @brief Interface for renderable 3D objects in the graphics system.
 *
 * This interface defines the common operations required for any object that can be rendered,
 * including initialization, rendering, cleanup, and material property management.
 */

#pragma once

#include "pch.h"
#include "Buffer.hpp"
#include "Lighting.hpp"

class Shader;

class IRenderable 
{
public:
    /**
     * @brief Virtual destructor for the renderable interface.
     */
    virtual ~IRenderable() = default;
    
    /**
     * @brief Initializes the renderable object with a shader.
     * @param shader Shared pointer to the shader program
     */
    virtual void Initialize(const std::shared_ptr<Shader>& shader) = 0;
    
    /**
     * @brief Renders the object with the given transformation matrices.
     * @param modelMatrix Model transformation matrix
     * @param viewMatrix View transformation matrix
     * @param projectionMatrix Projection transformation matrix
     */
    virtual void Render(const glm::mat4& modelMatrix, 
                       const glm::mat4& viewMatrix, 
                       const glm::mat4& projectionMatrix) = 0;
    
    /**
     * @brief Cleans up OpenGL resources.
     */
    virtual void CleanUp() = 0;
    
    /**
     * @brief Sets the material properties for this renderable.
     * @param material Material properties to apply
     */
    virtual void SetMaterial(const Material& material) { m_Material = material; }
    
    /**
     * @brief Gets the current material properties.
     * @return Current material properties
     */
    virtual Material GetMaterial() const { return m_Material; }
    
    /**
     * @brief Gets an editable reference to the material properties.
     * @return Editable reference to material properties
     */
    virtual Material& GetMaterialEditable() { return m_Material; }

protected:
    Buffer m_Buffer;
    std::shared_ptr<Shader> m_Shader;
    glm::mat4 m_ModelMatrix{};
    Material m_Material; // Default material (white)
}; 