#pragma once

#include <glm/glm.hpp>

/**
 * @struct DirectionalLight
 * @brief Represents a directional light with direction and color properties
 */
struct DirectionalLight {
    glm::vec3 direction;    ///< Light direction (world space)
    glm::vec3 ambient;      ///< Ambient intensity
    glm::vec3 diffuse;      ///< Diffuse intensity
    glm::vec3 specular;     ///< Specular intensity
    
    /**
     * @brief Default constructor with standard values
     */
    DirectionalLight()
        : direction(0.0f, -1.0f, 0.0f),
          ambient(0.2f, 0.2f, 0.2f),
          diffuse(0.5f, 0.5f, 0.5f),
          specular(1.0f, 1.0f, 1.0f)
    {}
    
    /**
     * @brief Parameterized constructor
     * @param dir Light direction
     * @param amb Ambient color
     * @param diff Diffuse color
     * @param spec Specular color
     */
    DirectionalLight(const glm::vec3& dir, const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec)
        : direction(glm::normalize(dir)),
          ambient(amb),
          diffuse(diff),
          specular(spec)
    {}
    
    /**
     * @brief Set light uniforms in the shader
     * @param shader Shader program ID
     */
    void setUniforms(unsigned int shader) const;
};

/**
 * @struct Material
 * @brief Represents material properties for lighting calculations
 */
struct Material {
    glm::vec3 ambient;     ///< Ambient color
    glm::vec3 diffuse;     ///< Diffuse color
    glm::vec3 specular;    ///< Specular color
    float shininess;      ///< Shininess factor
    
    /**
     * @brief Default constructor with standard values
     */
    Material()
        : ambient(1.0f),
          diffuse(1.0f),
          specular(0.5f),
          shininess(32.0f)
    {}
    
    /**
     * @brief Parameterized constructor
     * @param amb Ambient color
     * @param diff Diffuse color
     * @param spec Specular color
     * @param shin Shininess factor
     */
    Material(const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec, float shin)
        : ambient(amb),
          diffuse(diff),
          specular(spec),
          shininess(shin)
    {}
    
    /**
     * @brief Set material uniforms in the shader
     * @param shader Shader program ID
     */
    void setUniforms(unsigned int shader) const;
}; 