#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

/**
 * @class Shader
 * @brief Handles loading, compiling, and using shader programs
 */
class Shader {
public:
    /**
     * @brief Constructor that loads and compiles shaders from files
     * @param vertexPath Path to the vertex shader file
     * @param fragmentPath Path to the fragment shader file
     */
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    
    /**
     * @brief Destructor
     */
    ~Shader();
    
    /**
     * @brief Use this shader program
     */
    void use() const;
    
    /**
     * @brief Get the shader program ID
     * @return The program ID
     */
    unsigned int getId() const;
    
    // Uniform setter methods
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    
private:
    unsigned int m_programID;  ///< Shader program ID
    mutable std::unordered_map<std::string, int> m_uniformLocationCache;  ///< Cache for uniform locations
    
    /**
     * @brief Get uniform location with caching
     * @param name The name of the uniform
     * @return The uniform location
     */
    int getUniformLocation(const std::string& name) const;
    
    /**
     * @brief Check shader or program for compilation/linking errors
     * @param shader Shader or program ID
     * @param type Type of check ("VERTEX", "FRAGMENT", or "PROGRAM")
     */
    void checkCompileErrors(unsigned int shader, const std::string& type);
}; 