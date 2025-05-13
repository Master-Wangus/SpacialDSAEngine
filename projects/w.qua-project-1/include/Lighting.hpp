#pragma once

#include <glm/glm.hpp>

struct DirectionalLight {
    glm::vec4 direction;    ///< Light direction (world space) - vec4 for alignment
    glm::vec4 color;        ///< Light color - vec4 for alignment
    DirectionalLight()
        : direction(0.0f, -1.0f, 0.0f, 0.0f),
          color(1.0f, 1.0f, 1.0f, 1.0f)
          {}

    DirectionalLight(const glm::vec3& dir, const glm::vec4& col)
        : direction(glm::normalize(dir), 0.0f),
          color(col) 
          {}
    
    DirectionalLight(const glm::vec3& dir, const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec)
        : direction(glm::normalize(dir), 0.0f),
          color(1.0f) // Default color is white
    {
        // Store ambient, diffuse, specular components separately
        ambient = amb;
        diffuse = diff;
        specular = spec;
    }
    
    // We keep these for backward compatibility with existing code
    glm::vec3 ambient = glm::vec3(0.2f);  ///< Ambient intensity
    glm::vec3 diffuse = glm::vec3(0.5f);  ///< Diffuse intensity
    glm::vec3 specular = glm::vec3(1.0f); ///< Specular intensity
    
    void setUniforms(unsigned int shader) const;
};

struct Material {
    glm::vec3 ambientColor;       ///< Ambient color
    float ambientIntensity;       ///< Ambient intensity (for alignment)
    
    glm::vec3 diffuseColor;       ///< Diffuse color
    float diffuseIntensity;       ///< Diffuse intensity (for alignment)
    
    glm::vec3 specularColor;      ///< Specular color
    float specularIntensity;      ///< Specular intensity (for alignment)
    
    float shininess;              ///< Shininess factor
    glm::vec3 padding;            ///< Padding for alignment
    
    Material()
        : ambientColor(1.0f),
          ambientIntensity(1.0f),
          diffuseColor(1.0f),
          diffuseIntensity(1.0f),
          specularColor(0.5f),
          specularIntensity(1.0f),
          shininess(32.0f),
          padding(0.0f)
    {}
    
    Material(const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec, float shin)
        : ambientColor(amb),
          ambientIntensity(1.0f),
          diffuseColor(diff),
          diffuseIntensity(1.0f),
          specularColor(spec),
          specularIntensity(1.0f),
          shininess(shin),
          padding(0.0f)
    {
        // For backward compatibility
        ambient = amb;
        diffuse = diff;
        specular = spec;
    }
    
    // We keep these for backward compatibility with existing code
    glm::vec3 ambient;  ///< Ambient color
    glm::vec3 diffuse;  ///< Diffuse color
    glm::vec3 specular; ///< Specular color
    
    void setUniforms(unsigned int shader) const;
}; 