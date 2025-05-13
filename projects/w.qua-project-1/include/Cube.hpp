#pragma once

#include "Buffer.hpp"
#include "Lighting.hpp"
#include "Primitives.hpp"
#include <vector>
#include <memory>

class Shader; // Forward declaration

class Cube 
{
public:
    Cube(const glm::vec3& position = glm::vec3(0.0f), 
         float size = 1.0f,
         const glm::vec3& color = glm::vec3(1.0f));
    
    ~Cube();
    
    void initialize(const std::shared_ptr<Shader>& shader);
    void render(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
    AABB getAABB() const;
    BoundingSphere getBoundingSphere() const;
    void setPosition(const glm::vec3& position);
    glm::vec3 getPosition() const;
    void setMaterial(const Material& material);
    
private:
    glm::vec3 m_position;    ///< Position of the cube
    float m_size;            ///< Size of the cube (edge length)
    glm::vec3 m_color;       ///< Color of the cube
    Material m_material;     ///< Material properties
    glm::mat4 m_modelMatrix; ///< Model transformation matrix
    
    Buffer m_buffer;         ///< Buffer for geometry data
    std::shared_ptr<Shader> m_shader; ///< Shader program
    
    std::vector<Vertex> createVertices();
    void updateModelMatrix();
}; 