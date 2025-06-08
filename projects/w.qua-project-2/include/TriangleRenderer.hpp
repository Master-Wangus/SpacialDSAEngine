/**
 * @class TriangleRenderer
 * @brief Renderer for 3D triangle primitives with vertex color support.
 *
 * This class implements the IRenderable interface to render 3D triangles using OpenGL.
 * It manages triangle vertex data, normal calculation, and provides efficient rendering capabilities.
 */

#pragma once

#include "pch.h"
#include "Buffer.hpp"
#include "IRenderable.hpp"

class Shader;

class TriangleRenderer : public IRenderable
{
public:
    /**
     * @brief Constructs a triangle renderer with specified vertices and color.
     * @param v0 First vertex position
     * @param v1 Second vertex position
     * @param v2 Third vertex position
     * @param color Color of the triangle
     */
    TriangleRenderer(const glm::vec3& v0 = glm::vec3(0.0f, 0.0f, 0.0f),
                    const glm::vec3& v1 = glm::vec3(1.0f, 0.0f, 0.0f),
                    const glm::vec3& v2 = glm::vec3(0.0f, 1.0f, 0.0f),
                    const glm::vec3& color = glm::vec3(0.0f, 0.8f, 0.0f));
    
    /**
     * @brief Destructor for the triangle renderer.
     */
    ~TriangleRenderer();
    
    /**
     * @brief Initializes the triangle renderer with a shader.
     * @param shader Shared pointer to the shader program
     */
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    /**
     * @brief Renders the triangle with the given transformation matrices.
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
    
    // Triangle-specific functionality
    /**
     * @brief Sets the vertices of the triangle.
     * @param v0 First vertex position
     * @param v1 Second vertex position
     * @param v2 Third vertex position
     */
    void SetVertices(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2);
    
    /**
     * @brief Gets the vertices of the triangle.
     * @param v0 Output first vertex position
     * @param v1 Output second vertex position
     * @param v2 Output third vertex position
     */
    void GetVertices(glm::vec3& v0, glm::vec3& v1, glm::vec3& v2) const;
    
    /**
     * @brief Sets the color of the triangle.
     * @param color New color
     */
    void SetColor(const glm::vec3& color);
    
    /**
     * @brief Gets the color of the triangle.
     * @return Current color
     */
    glm::vec3 GetColor() const;
    
private:
    glm::vec3 m_Vertices[3];
    glm::vec3 m_Color;

    /**
     * @brief Creates vertex data for triangle rendering.
     * @return Vector of vertex data
     */
    std::vector<Vertex> CreateVertices();
    
    /**
     * @brief Computes the normal vector for the triangle.
     * @return Normal vector
     */
    glm::vec3 ComputeNormal() const;
}; 