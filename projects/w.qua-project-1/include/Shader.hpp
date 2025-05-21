#pragma once

#include "pch.h"

class Shader 
{
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();
    
    void Use() const;
    unsigned int GetID() const;
    
    // Uniform setter methods
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetMat2(const std::string& name, const glm::mat2& mat) const;
    void SetMat3(const std::string& name, const glm::mat3& mat) const;
    void SetMat4(const std::string& name, const glm::mat4& mat) const;
    
private:
    unsigned int m_ID;  ///< Shader program ID
    mutable std::unordered_map<std::string, int> m_uniformLocationCache;  ///< Cache for uniform locations
    
    int GetUniformLocation(const std::string& name) const;
    void CheckCompileErrors(unsigned int shader, const std::string& type);
}; 