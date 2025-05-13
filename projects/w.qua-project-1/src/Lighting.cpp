#include "../include/Lighting.hpp"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

void DirectionalLight::setUniforms(unsigned int shader) const {
    // Set light properties in shader
    glUseProgram(shader);
    glUniform3fv(glGetUniformLocation(shader, "dirLight.direction"), 1, glm::value_ptr(direction));
    glUniform3fv(glGetUniformLocation(shader, "dirLight.ambient"), 1, glm::value_ptr(ambient));
    glUniform3fv(glGetUniformLocation(shader, "dirLight.diffuse"), 1, glm::value_ptr(diffuse));
    glUniform3fv(glGetUniformLocation(shader, "dirLight.specular"), 1, glm::value_ptr(specular));
}

void Material::setUniforms(unsigned int shader) const {
    // Set material properties in shader
    glUseProgram(shader);
    glUniform3fv(glGetUniformLocation(shader, "material.ambient"), 1, glm::value_ptr(ambient));
    glUniform3fv(glGetUniformLocation(shader, "material.diffuse"), 1, glm::value_ptr(diffuse));
    glUniform3fv(glGetUniformLocation(shader, "material.specular"), 1, glm::value_ptr(specular));
    glUniform1f(glGetUniformLocation(shader, "material.shininess"), shininess);
} 