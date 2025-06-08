/**
 * @class PlaneRenderer
 * @brief Renderer for 3D plane primitives with custom size and appearance.
 *
 * This class implements the IRenderable interface to render 3D planes using OpenGL.
 * It provides efficient plane geometry generation and rendering with proper normal and texture coordinate support.
 */

#pragma once

#include "pch.h"
#include "Buffer.hpp"
#include "IRenderable.hpp"

class Shader;

class PlaneRenderer : public IRenderable
{
public:
    /**
     * @brief Constructs a plane renderer with specified properties.
     * @param normal Normal vector of the plane
     * @param distance Distance from origin along the normal
     * @param color Color of the plane
     * @param size Size of the plane
     */
    PlaneRenderer(const glm::vec3& normal = glm::vec3(0.0f, 1.0f, 0.0f), 
                 float distance = 0.0f,
                 const glm::vec3& color = glm::vec3(0.8f, 0.8f, 0.8f), 
                 float size = 10.0f);
    
    /**
     * @brief Destructor for the plane renderer.
     */
    ~PlaneRenderer();
    
    /**
     * @brief Initializes the plane renderer with a shader.
     * @param shader Shared pointer to the shader program
     */
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    /**
     * @brief Renders the plane with the given transformation matrices.
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
    
    // Plane-specific functionality
    /**
     * @brief Sets the normal vector of the plane.
     * @param normal New normal vector
     */
    void SetNormal(const glm::vec3& normal);
    
    /**
     * @brief Gets the normal vector of the plane.
     * @return Current normal vector
     */
    glm::vec3 GetNormal() const;
    
    /**
     * @brief Sets the distance from origin along the normal.
     * @param distance New distance value
     */
    void SetDistance(float distance);
    
    /**
     * @brief Gets the distance from origin along the normal.
     * @return Current distance value
     */
    float GetDistance() const;
    
    /**
     * @brief Sets the color of the plane.
     * @param color New color
     */
    void SetColor(const glm::vec3& color);
    
    /**
     * @brief Gets the color of the plane.
     * @return Current color
     */
    glm::vec3 GetColor() const;
    
    /**
     * @brief Sets the size of the plane.
     * @param size New size value
     */
    void SetSize(float size);
    
    /**
     * @brief Gets the size of the plane.
     * @return Current size value
     */
    float GetSize() const;
    
private:
    glm::vec3 m_Normal;
    float m_Distance;
    float m_Size;
    glm::vec3 m_Color;

    /**
     * @brief Creates vertex data for plane rendering.
     * @return Vector of vertex data
     */
    std::vector<Vertex> CreateVertices();
}; 