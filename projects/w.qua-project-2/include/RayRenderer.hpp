/**
 * @class RayRenderer
 * @brief Renderer for 3D ray primitives with origin, direction and length.
 *
 * This class implements the IRenderable interface to visualize rays in 3D space.
 * It provides line-based rendering of rays with customizable appearance for debugging and visualization.
 */

#pragma once

#include "pch.h"
#include "Buffer.hpp"
#include "IRenderable.hpp"

class Shader;

class RayRenderer : public IRenderable
{
public:
    /**
     * @brief Constructs a ray renderer with specified properties.
     * @param origin Origin point of the ray
     * @param direction Direction vector of the ray
     * @param color Color of the ray
     * @param length Length of the ray
     */
    RayRenderer(const glm::vec3& origin = glm::vec3(0.0f), 
               const glm::vec3& direction = glm::vec3(0.0f, 0.0f, 1.0f),
               const glm::vec3& color = glm::vec3(1.0f, 1.0f, 0.0f),
               float length = 10.0f);
    
    /**
     * @brief Destructor for the ray renderer.
     */
    ~RayRenderer();
    
    /**
     * @brief Initializes the ray renderer with a shader.
     * @param shader Shared pointer to the shader program
     */
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    /**
     * @brief Renders the ray with the given transformation matrices.
     * @param modelMatrix Model transformation matrix
     * @param viewMatrix View transformation matrix
     * @param projectionMatrix Projection transformation matrix
     */
    void Render(const glm::mat4& modelMatrix, 
               const glm::mat4& viewMatrix, 
               const glm::mat4& projectionMatrix) override;
    
    /**
     * @brief Cleans up OpenGL resources.
     */
    void CleanUp() override;
    
    // Ray-specific functionality
    /**
     * @brief Sets the origin point of the ray.
     * @param origin New origin point
     */
    void SetOrigin(const glm::vec3& origin);
    
    /**
     * @brief Gets the origin point of the ray.
     * @return Current origin point
     */
    glm::vec3 GetOrigin() const;
    
    /**
     * @brief Sets the direction vector of the ray.
     * @param direction New direction vector
     */
    void SetDirection(const glm::vec3& direction);
    
    /**
     * @brief Gets the direction vector of the ray.
     * @return Current direction vector
     */
    glm::vec3 GetDirection() const;
    
    /**
     * @brief Sets the color of the ray.
     * @param color New color
     */
    void SetColor(const glm::vec3& color);
    
    /**
     * @brief Gets the color of the ray.
     * @return Current color
     */
    glm::vec3 GetColor() const;
    
    /**
     * @brief Sets the length of the ray.
     * @param length New length value
     */
    void SetLength(float length);
    
    /**
     * @brief Gets the length of the ray.
     * @return Current length value
     */
    float GetLength() const;
    
    /**
     * @brief Sets the thickness of the ray.
     * @param thickness New thickness value
     */
    void SetThickness(float thickness);
    
    /**
     * @brief Gets the thickness of the ray.
     * @return Current thickness value
     */
    float GetThickness() const;
    
private:
    glm::vec3 m_Origin;
    glm::vec3 m_Direction;
    glm::vec3 m_Color;
    float m_Length;
    float m_Thickness;

    /**
     * @brief Creates vertex data for ray rendering.
     * @return Vector of vertex data
     */
    std::vector<Vertex> CreateVertices();
}; 