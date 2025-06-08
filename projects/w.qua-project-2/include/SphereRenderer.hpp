/**
 * @class SphereRenderer
 * @brief Renderer for 3D sphere primitives with customizable properties.
 *
 * This class implements the IRenderable interface to render 3D spheres using OpenGL.
 * It manages vertex generation for sphere approximation, buffer handling, and rendering with proper shading.
 */

#pragma once

#include "pch.h"
#include "IRenderable.hpp"

class Shader;
class Buffer;
struct DirectionalLight;

class SphereRenderer : public IRenderable
{
public:
    /**
     * @brief Constructs a sphere renderer with basic parameters.
     * @param center Center position of the sphere
     * @param radius Radius of the sphere
     * @param color Color of the sphere
     */
    SphereRenderer(const glm::vec3& center = glm::vec3(0.0f), 
                  float radius = 1.0f,
                  const glm::vec3& color = glm::vec3(1.0f, 0.0f, 0.0f));
    
    /**
     * @brief Constructs a sphere renderer with wireframe option.
     * @param center Center position of the sphere
     * @param radius Radius of the sphere
     * @param color Color of the sphere
     * @param wireframe Whether to render in wireframe mode
     */
    SphereRenderer(const glm::vec3& center, 
                  float radius,
                  const glm::vec3& color,
                  bool wireframe);
    
    /**
     * @brief Destructor for the sphere renderer.
     */
    ~SphereRenderer();
    
    /**
     * @brief Initializes the sphere renderer with a shader.
     * @param shader Shared pointer to the shader program
     */
    void Initialize(const std::shared_ptr<Shader>& shader) override;
    
    /**
     * @brief Renders the sphere with the given transformation matrices.
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
    
    // Sphere-specific functionality
    /**
     * @brief Sets the center position of the sphere.
     * @param center New center position
     */
    void SetCenter(const glm::vec3& center);
    
    /**
     * @brief Gets the center position of the sphere.
     * @return Current center position
     */
    glm::vec3 GetCenter() const;
    
    /**
     * @brief Sets the radius of the sphere.
     * @param radius New radius value
     */
    void SetRadius(float radius);
    
    /**
     * @brief Gets the radius of the sphere.
     * @return Current radius value
     */
    float GetRadius() const;
    
    /**
     * @brief Sets the color of the sphere.
     * @param color New color
     */
    void SetColor(const glm::vec3& color);
    
    /**
     * @brief Gets the color of the sphere.
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
     * @brief Checks if the sphere is rendered in wireframe mode.
     * @return True if wireframe, false if solid
     */
    bool IsWireframe() const;
    
private:
    glm::vec3 m_Center;
    float m_Radius;
    glm::vec3 m_Color;
    bool m_Wireframe = false;

    /**
     * @brief Creates vertex data for solid sphere rendering.
     * @return Vector of vertex data
     */
    std::vector<Vertex> CreateVertices();
    
    /**
     * @brief Creates vertex data for wireframe sphere rendering.
     * @return Vector of vertex data for wireframe
     */
    std::vector<Vertex> CreateWireframeVertices();
}; 