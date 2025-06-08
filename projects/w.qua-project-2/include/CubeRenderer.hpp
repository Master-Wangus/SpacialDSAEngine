/**
 * @class CubeRenderer
 * @brief Renderer for 3D cube primitives with custom color and transformation.
 *
 * This class implements the IRenderable interface to render 3D cubes using OpenGL.
 * It manages vertex data, buffers, and rendering operations with proper shading support.
 */

#pragma once

#include "pch.h"
#include "IRenderable.hpp"

class Shader;
struct Vertex;

class CubeRenderer : public IRenderable
{
public:
    /**
     * @brief Constructs a cube renderer with basic parameters.
     * @param center Center position of the cube
     * @param size Size of the cube in each dimension
     * @param color Color of the cube
     */
    CubeRenderer(const glm::vec3& center = glm::vec3(0.0f),
                const glm::vec3& size = glm::vec3(1.0f),
                const glm::vec3& color = glm::vec3(1.0f));
    
    /**
     * @brief Constructs a cube renderer with wireframe option.
     * @param center Center position of the cube
     * @param size Size of the cube in each dimension
     * @param color Color of the cube
     * @param wireframe Whether to render in wireframe mode
     */
    CubeRenderer(const glm::vec3& center,
                const glm::vec3& size,
                const glm::vec3& color,
                bool wireframe);
    
    /**
     * @brief Constructs an oriented cube renderer for bounding boxes.
     * @param center Center position of the oriented cube
     * @param axes Array of 3 orientation axes
     * @param halfExtents Half-extents of the cube along each axis
     * @param color Color of the cube
     * @param wireframe Whether to render in wireframe mode
     */
    CubeRenderer(const glm::vec3& center,
                const glm::vec3 axes[3],
                const glm::vec3& halfExtents,
                const glm::vec3& color,
                bool wireframe = true);
    
    /**
     * @brief Destructor for the cube renderer.
     */
    ~CubeRenderer();
    
    /**
     * @brief Initializes the cube renderer with a shader.
     * @param shader Shared pointer to the shader program
     */
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    /**
     * @brief Renders the cube with the given transformation matrices.
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
    
    // Cube-specific functionality
    /**
     * @brief Sets the center position of the cube.
     * @param center New center position
     */
    void SetCenter(const glm::vec3& center);
    
    /**
     * @brief Gets the center position of the cube.
     * @return Current center position
     */
    glm::vec3 GetCenter() const;
    
    /**
     * @brief Sets the size of the cube.
     * @param size New size in each dimension
     */
    void SetSize(const glm::vec3& size);
    
    /**
     * @brief Gets the size of the cube.
     * @return Current size
     */
    glm::vec3 GetSize() const;
    
    /**
     * @brief Sets the color of the cube.
     * @param color New color
     */
    void SetColor(const glm::vec3& color);
    
    /**
     * @brief Gets the color of the cube.
     * @return Current color
     */
    glm::vec3 GetColor() const;
    
    // Wireframe control
    /**
     * @brief Sets whether to render in wireframe mode.
     * @param wireframe True for wireframe, false for solid
     */
    void SetWireframe(bool wireframe);
    
    /**
     * @brief Checks if the cube is rendered in wireframe mode.
     * @return True if wireframe, false if solid
     */
    bool IsWireframe() const;
    
    // Orientation control
    /**
     * @brief Sets the orientation axes for the cube.
     * @param axes Array of 3 orientation axes
     */
    void SetOrientation(const glm::vec3 axes[3]);
    
    /**
     * @brief Sets the half-extents of the oriented cube.
     * @param halfExtents Half-extents along each axis
     */
    void SetHalfExtents(const glm::vec3& halfExtents);
    
private:
    glm::vec3 m_Center;
    glm::vec3 m_Size;
    glm::vec3 m_Color;
    bool m_Wireframe = false;
    bool m_IsOriented = false;
    glm::vec3 m_Axes[3] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };
    glm::vec3 m_HalfExtents = glm::vec3(0.5f);

    /**
     * @brief Creates vertex data for solid cube rendering.
     * @return Vector of vertex data
     */
    std::vector<Vertex> CreateVertices();
    
    /**
     * @brief Creates vertex data for wireframe cube rendering.
     * @return Vector of vertex data for wireframe
     */
    std::vector<Vertex> CreateWireframeVertices();
    
    /**
     * @brief Creates vertex data for oriented wireframe cube rendering.
     * @return Vector of vertex data for oriented wireframe
     */
    std::vector<Vertex> CreateOrientedWireframeVertices();
}; 