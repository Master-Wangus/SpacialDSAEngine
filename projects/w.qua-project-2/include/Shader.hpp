/**
 * @class Shader
 * @brief GLSL shader program manager for loading, compiling and using shaders.
 *
 * This class handles the complete lifecycle of OpenGL shader programs,
 * including compilation, linking, uniform setting, and runtime management.
 */

#pragma once

#include "pch.h"

class Shader 
{
public:
    /**
     * @brief Constructs a shader program from vertex and fragment shader files.
     * @param vertexPath Path to the vertex shader source file
     * @param fragmentPath Path to the fragment shader source file
     */
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    
    /**
     * @brief Destructor that cleans up the shader program.
     */
    ~Shader();
    
    /**
     * @brief Activates this shader program for rendering.
     */
    void Use() const;
    
    /**
     * @brief Gets the OpenGL shader program ID.
     * @return The OpenGL program ID
     */
    unsigned int GetID() const;
    
    // Uniform setter methods
    /**
     * @brief Sets a boolean uniform variable.
     * @param name Name of the uniform variable
     * @param value Boolean value to set
     */
    void SetBool(const std::string& name, bool value) const;
    
    /**
     * @brief Sets an integer uniform variable.
     * @param name Name of the uniform variable
     * @param value Integer value to set
     */
    void SetInt(const std::string& name, int value) const;
    
    /**
     * @brief Sets a float uniform variable.
     * @param name Name of the uniform variable
     * @param value Float value to set
     */
    void SetFloat(const std::string& name, float value) const;
    
    /**
     * @brief Sets a vec2 uniform variable.
     * @param name Name of the uniform variable
     * @param value vec2 value to set
     */
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    
    /**
     * @brief Sets a vec3 uniform variable.
     * @param name Name of the uniform variable
     * @param value vec3 value to set
     */
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    
    /**
     * @brief Sets a vec4 uniform variable.
     * @param name Name of the uniform variable
     * @param value vec4 value to set
     */
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    
    /**
     * @brief Sets a mat2 uniform variable.
     * @param name Name of the uniform variable
     * @param mat mat2 matrix to set
     */
    void SetMat2(const std::string& name, const glm::mat2& mat) const;
    
    /**
     * @brief Sets a mat3 uniform variable.
     * @param name Name of the uniform variable
     * @param mat mat3 matrix to set
     */
    void SetMat3(const std::string& name, const glm::mat3& mat) const;
    
    /**
     * @brief Sets a mat4 uniform variable.
     * @param name Name of the uniform variable
     * @param mat mat4 matrix to set
     */
    void SetMat4(const std::string& name, const glm::mat4& mat) const;
    
private:
    unsigned int m_ID;  ///< Shader program ID
    mutable std::unordered_map<std::string, int> m_uniformLocationCache;  ///< Cache for uniform locations
    
    /**
     * @brief Gets the location of a uniform variable, with caching.
     * @param name Name of the uniform variable
     * @return Location of the uniform variable
     */
    int GetUniformLocation(const std::string& name) const;
    
    /**
     * @brief Checks for shader compilation or linking errors.
     * @param shader Shader ID to check
     * @param type Type of shader or "PROGRAM" for program linking
     */
    void CheckCompileErrors(unsigned int shader, const std::string& type);
}; 