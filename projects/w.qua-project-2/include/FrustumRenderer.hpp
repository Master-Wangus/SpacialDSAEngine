/**
 * @class FrustumRenderer
 * @brief Renderer for view frustum visualization with wireframe display.
 *
 * This class implements the IRenderable interface to render view frustums using OpenGL.
 * It creates wireframe geometry for the 8 vertices and 12 edges of the frustum.
 */

#pragma once

#include "pch.h"
#include "IRenderable.hpp"

class Shader;
struct Vertex;

class FrustumRenderer : public IRenderable
{
public:
    /**
     * @brief Constructs a frustum renderer with specified color.
     * @param color Color for the frustum wireframe (default magenta)
     */
    FrustumRenderer(const glm::vec3& color = glm::vec3(1.0f, 0.0f, 1.0f)); // Default magenta
    
    /**
     * @brief Destructor for the frustum renderer.
     */
    ~FrustumRenderer();
    
    /**
     * @brief Initializes the frustum renderer with a shader.
     * @param shader Shared pointer to the shader program
     */
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    /**
     * @brief Renders the frustum with the given transformation matrices.
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
    
    /**
     * @brief Updates the frustum geometry with a new view-projection matrix.
     * @param invViewProjection Inverse view-projection matrix defining the frustum
     */
    void UpdateFrustum(const glm::mat4& invViewProjection);
    
private:
    glm::vec3 m_Color;
    glm::mat4 m_InvViewProjection;
    bool m_FrustumDirty = true;

    /**
     * @brief Creates vertex data for the frustum wireframe.
     * @return Vector of vertex data for frustum edges
     */
    std::vector<Vertex> CreateFrustumVertices();
    
    /**
     * @brief Updates the vertex buffer with new frustum geometry.
     */
    void UpdateVertices();
}; 