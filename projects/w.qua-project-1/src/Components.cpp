#include "../include/Components.hpp"
#include <glm/gtc/matrix_transform.hpp>

void TransformComponent::updateModelMatrix() 
{
    // Start with identity matrix
    model = glm::mat4(1.0f);
    
    // Apply transformations in order: scale, rotate, translate
    model = glm::translate(model, position);
    
    // Apply rotations in order: x, y, z
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    
    model = glm::scale(model, scale);
} 