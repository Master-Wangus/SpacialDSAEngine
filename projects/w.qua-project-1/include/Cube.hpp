#pragma once

#include "Buffer.hpp"
#include "Lighting.hpp"
#include "Primitives.hpp"
#include <vector>
#include <memory>

class Shader; // Forward declaration

/**
 * @class Cube
 * @brief Represents a 3D cube with material properties and collision detection capabilities
 */
class Cube {
public:
    /**
     * @brief Constructor initializes the cube geometry and properties
     * @param position The position of the cube
     * @param size The size of the cube (edge length)
     * @param color The color of the cube
     */
    Cube(const glm::vec3& position = glm::vec3(0.0f), 
         float size = 1.0f,
         const glm::vec3& color = glm::vec3(1.0f));
    
    /**
     * @brief Destructor
     */
    ~Cube();
    
    /**
     * @brief Initializes cube's VAO, VBO, and other OpenGL resources
     * @param shader Pointer to the shader program to use
     */
    void initialize(const std::shared_ptr<Shader>& shader);
    
    /**
     * @brief Renders the cube
     * @param viewMatrix The view matrix
     * @param projMatrix The projection matrix
     */
    void render(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
    
    /**
     * @brief Gets the axis-aligned bounding box for this cube
     * @return AABB representation
     */
    AABB getAABB() const;
    
    /**
     * @brief Gets the bounding sphere for this cube
     * @return BoundingSphere representation
     */
    BoundingSphere getBoundingSphere() const;

    /**
     * @brief Sets the position of the cube
     * @param position New position
     */
    void setPosition(const glm::vec3& position);
    
    /**
     * @brief Gets the position of the cube
     * @return Current position
     */
    glm::vec3 getPosition() const;
    
    /**
     * @brief Sets the material properties of the cube
     * @param material New material
     */
    void setMaterial(const Material& material);
    
private:
    glm::vec3 m_position;    ///< Position of the cube
    float m_size;            ///< Size of the cube (edge length)
    glm::vec3 m_color;       ///< Color of the cube
    Material m_material;     ///< Material properties
    glm::mat4 m_modelMatrix; ///< Model transformation matrix
    
    Buffer m_buffer;         ///< Buffer for geometry data
    std::shared_ptr<Shader> m_shader; ///< Shader program
    
    /**
     * @brief Creates the cube vertices
     * @return Vector of vertices
     */
    std::vector<Vertex> createVertices();
    
    /**
     * @brief Updates the model matrix based on position and orientation
     */
    void updateModelMatrix();
}; 