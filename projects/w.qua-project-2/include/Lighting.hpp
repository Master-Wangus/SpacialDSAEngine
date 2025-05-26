/**
 * @file Lighting.hpp
 * @brief Definitions for lighting and material properties in 3D rendering.
 *
 * This file defines structures for representing directional lights and material properties,
 * with proper memory alignment for GPU buffer usage in lighting calculations.
 */

#pragma once

#include "pch.h"

struct DirectionalLight 
{
    glm::vec4 m_Direction;     ///< Light direction (world space)
    glm::vec4 m_Color;         ///< Light color and intensity
    float m_Enabled;           ///< Whether light is enabled (1.0f = enabled, 0.0f = disabled)
    glm::vec3 m_Padding;       ///< Padding for std140 layout
    
    DirectionalLight()
        : m_Direction(0.0f, -1.0f, 0.0f, 0.0f),
          m_Color(1.0f, 1.0f, 1.0f, 1.0f),
          m_Enabled(1.0f),
          m_Padding(0.0f)
          {}

    DirectionalLight(const glm::vec3& dir, const glm::vec4& col)
        : m_Direction(glm::normalize(dir), 0.0f),
          m_Color(col),
          m_Enabled(1.0f),
          m_Padding(0.0f)
          {}
    
    DirectionalLight(const glm::vec3& dir, const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec)
        : m_Direction(glm::normalize(dir), 0.0f),
          m_Color(1.0f), // Default color is white
          m_Enabled(1.0f),
          m_Padding(0.0f)
    {

    }
};

struct Material 
{
    glm::vec3 m_AmbientColor;       
    float m_AmbientIntensity;       
    
    glm::vec3 m_DiffuseColor;      
    float m_DiffuseIntensity;       
    
    glm::vec3 m_SpecularColor;     
    float m_SpecularIntensity;     

    float m_Shininess;            
    glm::vec3 m_Padding;            
    
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