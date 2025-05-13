#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();
    
    void use() const;
    unsigned int getId() const;
    unsigned int getID() const;
    
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
    unsigned int m_ID;  ///< Shader program ID
    mutable std::unordered_map<std::string, int> m_uniformLocationCache;  ///< Cache for uniform locations
    
    int getUniformLocation(const std::string& name) const;
    void checkCompileErrors(unsigned int shader, const std::string& type);
}; 