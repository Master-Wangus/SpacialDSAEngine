#pragma once

#include <glm/glm.hpp>

struct DirectionalLight 
{
    glm::vec4 m_Direction;    ///< Light direction (world space) - vec4 for alignment
    glm::vec4 m_Color;        ///< Light color - vec4 for alignment
    DirectionalLight()
        : m_Direction(0.0f, -1.0f, 0.0f, 0.0f),
          m_Color(1.0f, 1.0f, 1.0f, 1.0f)
          {}

    DirectionalLight(const glm::vec3& dir, const glm::vec4& col)
        : m_Direction(glm::normalize(dir), 0.0f),
          m_Color(col) 
          {}
    
    DirectionalLight(const glm::vec3& dir, const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec)
        : m_Direction(glm::normalize(dir), 0.0f),
          m_Color(1.0f) // Default color is white
    {

    }
};

struct Material 
{
    glm::vec3 m_AmbientColor;       ///< Ambient color
    float m_AmbientIntensity;       ///< Ambient intensity (for alignment)
    
    glm::vec3 m_DiffuseColor;       ///< Diffuse color
    float m_DiffuseIntensity;       ///< Diffuse intensity (for alignment)
    
    glm::vec3 m_SpecularColor;      ///< Specular color
    float m_SpecularIntensity;      ///< Specular intensity (for alignment)
    
    float m_Shininess;              ///< Shininess factor
    glm::vec3 m_Padding;            ///< Padding for alignment
    
    Material()
        : m_AmbientColor(1.0f),
          m_AmbientIntensity(1.0f),
          m_DiffuseColor(1.0f),
          m_DiffuseIntensity(1.0f),
          m_SpecularColor(0.5f),
          m_SpecularIntensity(1.0f),
          m_Shininess(32.0f),
          m_Padding(0.0f)
    {}
    
    Material(const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec, float shin)
        : m_AmbientColor(amb),
          m_AmbientIntensity(1.0f),
          m_DiffuseColor(diff),
          m_DiffuseIntensity(1.0f),
          m_SpecularColor(spec),
          m_SpecularIntensity(1.0f),
          m_Shininess(shin),
          m_Padding(0.0f)
    {
    }
    }; 